#include "AppCore.h" // Оставьте только один локальный include
#include "Common/Logger/Logger.h"
#include "Core/DataManager/DataManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/PipelineManager/PipelineManager.h"
#include "Core/TaskManager/TaskManager.h"
// Добавьте эти инклуды, если их нет, чтобы типы были известны
#include "Common/Enums/IOEnums.h"
#include "Common/Interfaces/IView.h"
#include "Enums/RenderEnums.h"
#include "Interfaces/IOFactory.h"
#include "Interfaces/IView.h"
#include "Visualize/VtkView.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <qfileinfo.h>
#include <qloggingcategory.h>
#include <quuid.h>

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
    if (m_isInitialized)
        return; // Защитный гвард
    connect(m_viewManager.get(), &Core::ViewManager::viewUpdateRequested, this, &AppCore::sceneUpdateRequested);
    connect(m_dataManager.get(), &DataManager::fileReady, this, &AppCore::onFileReady);
    connect(m_dataManager.get(), &DataManager::ioStarted, this, [this](const QUuid& taskId, const QString&, int total) {
        m_activeTasks[taskId] = total;
    });
    connect(m_dataManager.get(), &DataManager::ioFinished, this, [this](const QUuid& taskId, bool success) {
        if (success) {
            // Финальный рендер для пачки
            emit sceneUpdateRequested();
        }
        m_activeTasks.remove(taskId);
    });
    m_isInitialized = true;
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
            m_viewManager->forEachView([&](Visualize::IView* view) { m_layerManager->createLayer(node, view); });
            emit nodeAdded(node);
        }
    } else {
        m_objectRegistry->registerNode(node);
        m_viewManager->forEachView([&](Visualize::IView* view) { m_layerManager->createLayer(node, view); });
        emit nodeAdded(node);
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
std::shared_ptr<QSpace::Core::DataNode> AppCore::getNodeById(const QUuid& nodeId) {
    return m_objectRegistry->getNode(nodeId);
}
void AppCore::loadPalette(const QString& filePath) {
    auto colorMapOpt = m_sessionManager->loadPalette(filePath);
    if (colorMapOpt.has_value()) {
        emit paletteLoaded(colorMapOpt.value());
    } else {
        qCWarning(LogCore) << "Failed to load palette from:" << filePath;
    }
}
void AppCore::savePalette(const Visualize::ColorMap& map, const QString& filePath) {
    bool ok = m_sessionManager->savePalette(map, filePath);
    if (!ok) {
        qCWarning(LogCore) << "Failed to save palette to:" << filePath;
    }
}
void AppCore::removeLayer(const QString& layerName) {
    m_objectRegistry->removeObject(QUuid::fromString(layerName));
}
QUuid AppCore::getNodePaletteId(const QUuid& nodeId) {
    auto node = m_objectRegistry->getNode(nodeId);
    if (node) {
        return node->settings.colorMapId;
    }
    return QUuid();
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
        emit sceneUpdateRequested();
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
    saveCurrentProjectAs(m_session_state.projectFilePath);
}
void AppCore::saveCurrentProjectAs(const QString& projectPath) {
    if (projectPath.isEmpty())
        return;

    m_session_state.projectFilePath = projectPath;
    // Можно автоматически подставить имя файла как имя проекта, если оно пустое
    if (m_session_state.projectName.isEmpty()) {
        m_session_state.projectName = QFileInfo(projectPath).baseName();
    }

    bool ok = m_sessionManager->saveProject(m_session_state);
    if (ok) {
        m_session_state.isDirty = false;
        emit sessionStateChanged(m_session_state);

    } else {
        qCCritical(LogCore) << "Failed to save project to:" << projectPath;
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
    auto vtkView = m_viewManager->getView(m_viewManager->getMainViewId());
    if (!vtkView)
        return;

    // 1. Получаем базовый слой, который будем анимировать
    // Для этого нужно достать его из LayerManager (надеюсь, у тебя есть метод вроде getLayer)
    auto targetLayer = m_layerManager->getLayer(baseNodeId, vtkView);
    if (!targetLayer) {
        qCCritical(LogCore) << "Cannot start export: target layer not found!";
        emit exportFinished(false);
        return;
    }

    // 2. Создаем экспортер
    auto exporter = std::make_shared<QSpace::Visualize::VideoExporter>(dynamic_cast<Visualize::VtkView*>(vtkView));
    if (!exporter->startExport(outputPath, fps)) { // 30 FPS
        emit exportFinished(false);
        return;
    }

    // 3. Создаем менеджер
    m_videoExportManager = std::make_shared<VideoExportManager>(m_dataManager.get(), targetLayer, exporter, this);

    // 4. Пробрасываем сигналы в UI
    connect(m_videoExportManager.get(), &VideoExportManager::progressUpdated, this, &AppCore::exportProgressUpdated);
    connect(m_videoExportManager.get(), &VideoExportManager::exportFinished, this, [this](bool success) {
        emit exportFinished(success); // Сначала уведомляем UI
        QTimer::singleShot(0, this, [this]() { m_videoExportManager.reset(); });
    });

    // Получаем формат из первого файла
    QSpace::IO::FileFormat        format = QSpace::IO::Utils::getFormat(files.first());
    QSpace::Visualize::EntityType type   = QSpace::IO::Utils::getEntityType(QFileInfo(files.first()).fileName());
    QSpace::IO::ReadScheme        scheme = QSpace::IO::SchemeFactory::createDefaultSheme(type, format);

    // 5. Погнали!
    m_videoExportManager->start(files, scheme, format, stride);
}
QUuid AppCore::createView(Visualize::ViewType type, vtkRenderWindow* existingWindow) {
    // TODO: исправить работу метода добавить поддержку 2D графиков
    if (type == QSpace::Visualize::ViewType::VTK_3D) {
        QUuid viewId = m_viewManager->createView(QSpace::Visualize::CameraViewType::Iso, existingWindow);

        if (!viewId.isNull()) {
            auto newView = m_viewManager->getView(viewId);
            for (const auto& node : m_objectRegistry->getAllNodes()) { // Предполагается, что такой метод есть
                m_layerManager->createLayer(node, newView);
            }
            emit viewCreated(viewId, type);
        }
        return viewId;
    }
    return QUuid();
}

void AppCore::removeView(const QUuid& viewId) {
    m_viewManager->removeView(viewId);
    emit viewRemoved(viewId);
}
void AppCore::cancelVideoExport() {
    if (m_videoExportManager) {
        m_videoExportManager->cancel();
    }
}
void AppCore::setGlobalExposureAllViews(double exposure) {
    m_viewManager->forEachView([exposure](Visualize::IView* r) { r->setGlobalExposure(exposure); });
}
void AppCore::setGlobalExposureView(const QUuid& viewId, double exposure) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->setGlobalExposure(exposure);
    }
}
void AppCore::resetCameraInAllViews() {
    m_viewManager->forEachView([](Visualize::IView* r) { r->resetCamera(); });
}
void AppCore::setCameraViewInAllViews(Visualize::CameraViewType viewType) {
    m_viewManager->forEachView([viewType](Visualize::IView* r) { r->setCameraView(viewType); });
}
void AppCore::setBackgroundColorInAllViews(float r, float g, float b) {
    m_viewManager->forEachView([r, g, b](Visualize::IView* rw) { rw->setBackgroundColor(r, g, b); });
}
void AppCore::setAxesVisibleInAllViews(bool visible) {
    m_viewManager->forEachView([visible](Visualize::IView* rw) { rw->setAxesVisible(visible); });
}
void AppCore::setGridVisibleInAllViews(bool visible) {
    m_viewManager->forEachView([visible](Visualize::IView* rw) { rw->setGridVisible(visible); });
}
void AppCore::resetCameraInView(const QUuid& viewId) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->resetCamera();
    }
}
void AppCore::setCameraViewInView(const QUuid& viewId, Visualize::CameraViewType viewType) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->setCameraView(viewType);
    }
}
void AppCore::setBackgroundColorInView(const QUuid& viewId, float r, float g, float b) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->setBackgroundColor(r, g, b);
    }
}
void AppCore::setAxesVisibleInView(const QUuid& viewId, bool visible) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->setAxesVisible(visible);
    }
}
void AppCore::setGridVisibleInView(const QUuid& viewId, bool visible) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->setGridVisible(visible);
    }
}
void AppCore::removeObject(const QUuid& id) {
    m_objectRegistry->removeObject(id);
    m_layerManager->removeLayer(id); // TODO:: проверить не удаляет ло он данные из реестра
    emit objectRemoved(id);
    emit sceneUpdateRequested();
}
} // namespace QSpace::Core