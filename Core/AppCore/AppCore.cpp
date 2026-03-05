#include "AppCore.h" // Оставьте только один локальный include
#include "Common/Logger/Logger.h"
#include "Core/DataManager/DataManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/PipelineManager/PipelineManager.h"
#include "Core/TaskManager/TaskManager.h"
// Добавьте эти инклуды, если их нет, чтобы типы были известны
#include "Common/Enums/IOEnums.h"
#include "Enums/RenderEnums.h"
#include "Interfaces/IOFactory.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <qfileinfo.h>

namespace QSpace::Core {
AppCore::AppCore(QObject* parent) : QObject(parent) {
    m_taskManager    = std::make_unique<TaskManager>();
    m_objectRegistry = std::make_unique<ObjectRegistry>();
    m_dataManager    = std::make_unique<DataManager>(m_taskManager.get());
    m_viewManager    = std::make_unique<ViewManager>();
    m_layerManager   = std::make_unique<LayerManager>();
    m_pipelineManager =
        std::make_unique<PipelineManager>(m_objectRegistry.get(), m_viewManager.get(), m_layerManager.get());
    m_sessionManager = std::make_unique<SessionManager>(m_objectRegistry.get(), this);
}
void AppCore::initialize() {
    m_viewManager->createView();
    connect(m_dataManager.get(), &DataManager::fileReady, this, &AppCore::onFileReady);
    connect(m_dataManager.get(), &DataManager::ioStarted, this, [this](const QUuid& taskId, const QString&, int total) {
        m_activeTasks[taskId] = total;
    });
    connect(m_dataManager.get(), &DataManager::ioFinished, this, [this](const QUuid& taskId, bool success) {
        if (m_activeTasks.value(taskId) > 1 && success) {
            // Финальный рендер для пачки
            m_viewManager->forEachView([](QSpace::Visualize::Renderer* r) {
                r->resetCamera();
                r->render();
            });
        }
        m_activeTasks.remove(taskId);
    });
}
// 3. Обновленный onFileReady
void AppCore::onFileReady(const QUuid& taskId, QSpace::IO::ReadResult result, QSpace::IO::ImportRole role) {
    if (role != QSpace::IO::ImportRole::ProjectData)
        return;

    if (!result.isSuccess()) {
        qCWarning(LogCore) << "Error loading: " << result.errMessage;
        // Если была ошибка, удаляем из очереди восстановления
        m_restoringNodes.remove(result.path);
        return;
    }

    int                           totalInThisTask = m_activeTasks.value(taskId, 1);
    QString                       fileName        = QFileInfo(result.path).fileName();
    QSpace::Visualize::EntityType type            = QSpace::IO::Utils::getEntityType(result.path);

    // Создаем базовую ноду
    auto node = std::make_shared<DataNode>(result.data, fileName, type);
    // --- МАГИЯ ВОССТАНОВЛЕНИЯ ---
    // Если этот файл грузится из сохраненного проекта, подменяем данные!
    if (m_restoringNodes.contains(result.path)) {
        auto restoredState = m_restoringNodes.take(result.path); // Забираем и удаляем из мапы

        node->id       = restoredState.id;       // Возвращаем старый UUID!
        node->label    = restoredState.label;    // Возвращаем переименованное имя (если было)
        node->settings = restoredState.settings; // Возвращаем цвета, прозрачность и т.д.
        node->path     = restoredState.path;
        node->format   = restoredState.format;
        node->scheme   = restoredState.scheme;
        node->type     = restoredState.type;
        // Схему парсинга перезаписывать не нужно, она уже отработала в DataManager
    } else {
        // Обычная логика для новых файлов
        node->settings.isVisible = (totalInThisTask == 1);
        node->path               = result.path;
        node->format             = result.format;
        node->scheme = result.scheme; // TODO:: внимание существует проблемма при невалидной схеме приложение падает
        m_session_state.isDirty = true;
        emit sessionStateChanged(m_session_state);
    }
    // ----------------------------

    qCInfo(LogCore) << "AppCore::onFileReady - File loaded:" << fileName << "Points:" << node->stats.pointCount;

    if (m_autoGrouping) {
        QString groupName = extractGroupName(fileName);
        if (!groupName.isEmpty()) {
            auto container = findOrCreateContainer(groupName);
            container->addComponent(node);
            m_objectRegistry->registerNode(node);
            qInfo() << "Grouped" << fileName << "into" << groupName;
        } else {
            m_objectRegistry->registerNode(node);
        }
    } else {
        m_objectRegistry->registerNode(node);
    }
}
void AppCore::importFiles(const QStringList& paths) {
    if (paths.isEmpty())
        return;
    if (paths.size() == 1) {
        QSpace::IO::FileFormat        format     = QSpace::IO::Utils::getFormat(paths[0]);
        QSpace::Visualize::EntityType entityType = QSpace::IO::Utils::getEntityType(QFileInfo(paths[0]).fileName());
        QSpace::IO::ReadScheme        scheme     = QSpace::IO::SchemeFactory::createDefaultSheme(entityType, format);
        m_dataManager->importDataAsync(paths[0], scheme);
    } else {
        QList<QSpace::IO::BatchTask> tasks;
        for (const auto& path : paths) {
            QSpace::IO::FileFormat        format     = QSpace::IO::Utils::getFormat(QFileInfo(path).fileName());
            QSpace::Visualize::EntityType entityType = QSpace::IO::Utils::getEntityType(QFileInfo(path).fileName());
            QSpace::IO::ReadScheme        scheme = QSpace::IO::SchemeFactory::createDefaultSheme(entityType, format);
            QSpace::IO::BatchTask         task;
            task.path   = path;
            task.scheme = scheme;
            tasks.append(task);
        }
        m_dataManager->importBatchDataAsync(tasks);
    }
}
void AppCore::removeLayer(const QString& layerName) {
    m_objectRegistry->removeObject(QUuid::fromString(layerName));
}

std::shared_ptr<DataContainer> AppCore::findOrCreateContainer(const QString& groupName) {
    auto existing = m_objectRegistry->findContainerByName(groupName);
    if (existing)
        return existing;

    auto newContainer = std::make_shared<DataContainer>(groupName);
    m_objectRegistry->registerContainer(newContainer);
    return newContainer;
}

QString AppCore::extractGroupName(const QString& filename) {
    // Ищем паттерны вида "snap_001", "step-500"
    static QRegularExpression regex(R"((snap(shot)?|step)[_\-]?\d+)", QRegularExpression::CaseInsensitiveOption);
    auto                      match = regex.match(filename);
    return match.hasMatch() ? match.captured(0) : QString();
}

void AppCore::updateNodeSettings(const QUuid& id, std::function<void(VisualSettings&)> modifer) {
    auto node = m_objectRegistry->getNode(id);
    if (node) {
        modifer(node->settings);
        m_layerManager->updateSettings(id);
    }
}
void AppCore::createNewProject(const QString& ProjectName) {
    m_objectRegistry->clear();
    m_session_state.projectName = ProjectName;
    m_session_state.isDirty     = false;
    emit sessionStateChanged(m_session_state);
}
void AppCore::saveCurrentProject() {
    if (m_session_state.projectFilePath.isEmpty()) {
        emit requestSavePathFromUI();
        return;
    }
    bool ok = m_sessionManager->saveProject(m_session_state);
    if (ok) {
        m_session_state.isDirty = false;
        emit sessionStateChanged(m_session_state);
    }
}
// 2. Обновленный openProject
void AppCore::openProject(const QString& projectPath) {
    auto newState = m_sessionManager->loadProject(projectPath);
    if (newState.has_value()) {
        m_session_state.projectName     = newState->projectName;
        m_session_state.projectFilePath = projectPath;
        m_session_state.isDirty         = false;

        // Очищаем текущий проект
        m_objectRegistry->clear();
        m_restoringNodes.clear();

        for (const auto& nodeState : newState->nodesStates) {
            // Сохраняем стейт в мапу по пути к файлу
            m_restoringNodes.insert(nodeState.path, nodeState);

            // Запускаем асинхронное чтение
            m_dataManager->importDataAsync(nodeState.path, nodeState.scheme);
        }
    } else {
        qCCritical(LogCore) << "The project has not been opened: " << projectPath;
    }
}
void AppCore::startVideoExport(const QUuid&       baseNodeId,
                               const QStringList& files,
                               const QString&     outputPath,
                               int                stride,
                               int                fps) {
    auto renderer = m_viewManager->getView(m_viewManager->getMainViewId());
    if (!renderer)
        return;

    // 1. Получаем базовый слой, который будем анимировать
    // Для этого нужно достать его из LayerManager (надеюсь, у тебя есть метод вроде getLayer)
    auto targetLayer = m_layerManager->getLayer(baseNodeId, renderer);
    if (!targetLayer) {
        qCCritical(LogCore) << "Cannot start export: target layer not found!";
        emit exportFinished(false);
        return;
    }

    // 2. Создаем экспортер
    auto exporter = std::make_shared<QSpace::Visualize::VideoExporter>(renderer);
    if (!exporter->startExport(outputPath, fps)) { // 30 FPS
        emit exportFinished(false);
        return;
    }

    // 3. Создаем менеджер
    m_videoExportManager = std::make_shared<VideoExportManager>(m_dataManager.get(), targetLayer, exporter, this);

    // 4. Пробрасываем сигналы в UI
    connect(m_videoExportManager.get(), &VideoExportManager::progressUpdated, this, &AppCore::exportProgressUpdated);
    connect(m_videoExportManager.get(), &VideoExportManager::exportFinished, this, [this](bool success) {
        m_videoExportManager.reset(); // Очищаем память после завершения
        emit exportFinished(success);
    });

    // Получаем формат из первого файла
    QSpace::IO::FileFormat        format = QSpace::IO::Utils::getFormat(files.first());
    QSpace::Visualize::EntityType type   = QSpace::IO::Utils::getEntityType(QFileInfo(files.first()).fileName());
    QSpace::IO::ReadScheme        scheme = QSpace::IO::SchemeFactory::createDefaultSheme(type, format);

    // 5. Погнали!
    m_videoExportManager->start(files, scheme, format, stride);
}

void AppCore::cancelVideoExport() {
    if (m_videoExportManager) {
        m_videoExportManager->cancel();
    }
}
} // namespace QSpace::Core