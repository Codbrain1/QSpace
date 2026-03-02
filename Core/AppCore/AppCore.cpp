#include "Core/AppCore/AppCore.h"
#include "AppCore.h"
#include "Common/Logger/Logger.h"
#include "Core/DataManager/DataManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/PipelineManager/PipelineManager.h"
#include "Core/TaskManager/TaskManager.h"
#include "Enums/CommonEnumsIO.h"
#include "Enums/RenderEnums.h"
#include "Interfaces/IOFactory.h"
#include "Structures/CoreStructures.h"
#include <functional>
#include <memory>
#include <qcontainerfwd.h>
#include <qfiledevice.h>
#include <qfileinfo.h>
#include <qloggingcategory.h>
#include <qobject.h>
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
            m_viewManager->forEachView([](Visualize::Renderer* r) {
                r->resetCamera();
                r->render();
            });
        }
        m_activeTasks.remove(taskId);
    });
}
void AppCore::onFileReady(const QUuid& taskId, IO::ReadResult result, IO::ImportRole role) {
    if (role != IO::ImportRole::ProjectData)
        return;
    if (!result.isSuccess()) {
        qCWarning(LogCore) << "Error loading: " << result.errMessage;
        return;
    }
    int                   totalInThisTask = m_activeTasks.value(taskId, 1);
    QString               fileName        = QFileInfo(result.path).fileName();
    Visualize::EntityType type            = IO::Utils::getEntityType(result.path);
    auto                  node            = std::make_shared<DataNode>(result.data, fileName, type);
    qCInfo(LogCore) << "AppCore::onFileReady - File loaded:" << fileName << "Points:" << node->stats.pointCount;
    node->settings.isVisible = (totalInThisTask == 1);

    if (m_autoGrouping) {
        QString groupName = extractGroupName(fileName);
        if (!groupName.isEmpty()) {
            auto container = findOrCreateContainer(groupName);
            container->addComponent(node);
            // Регистрируем ноду (она вызовет signal nodeAdded -> PipelineManager -> View)
            m_objectRegistry->registerNode(node);
            qInfo() << "Grouped" << fileName << "into" << groupName;
        } else {
            m_objectRegistry->registerNode(node);
        }
    } else {
        m_objectRegistry->registerNode(node);
    }

    qCInfo(LogCore) << "AppCore::onFileReady - File processing completed";
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
    auto exporter = std::make_shared<Visualize::VideoExporter>(renderer);
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
    IO::FileFormat        format = IO::Utils::getFormat(files.first());
    Visualize::EntityType type   = IO::Utils::getEntityType(QFileInfo(files.first()).fileName());
    IO::ReadScheme        scheme = IO::SchemeFactory::createDefaultSheme(type, format);

    // 5. Погнали!
    m_videoExportManager->start(files, scheme, format, stride);
}

void AppCore::cancelVideoExport() {
    if (m_videoExportManager) {
        m_videoExportManager->cancel();
    }
}
} // namespace QSpace::Core