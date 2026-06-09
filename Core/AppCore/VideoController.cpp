#include "VideoController.h"
#include "Common/Logger/Logger.h"
#include "Core/DataManager/DataManager.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/VideoExportManager/VideoExportManager.h"
#include "Core/ViewManager/ViewManager.h"
#include "Interfaces/IOFactory.h"
#include "Visualize/VideoExporter/VideoExporter.h"
#include "Visualize/VtkView.h"
#include <QFileInfo>
#include <QTimer>

namespace QSpace::Core::Controllers {

VideoController::VideoController(Core::DataManager*  dataManager,
                                 Core::ViewManager*  viewManager,
                                 Core::LayerManager* layerManager,
                                 QObject*            parent)
    : QObject(parent),
      m_dataManager(dataManager),
      m_viewManager(viewManager),
      m_layerManager(layerManager) {
}

void VideoController::startVideoExport(const QUuid&       baseNodeId,
                                       const QStringList& files,
                                       const QString&     outputPath,
                                       int                stride,
                                       int                fps) {
    if (files.isEmpty())
        return;

    auto vtkView = m_viewManager->getView(m_viewManager->getMainViewId());
    if (!vtkView) {
        qCCritical(LogCore) << "Cannot start export: main VTK view not found!";
        emit exportFinished(false);
        return;
    }

    // 1. Ищем слой, привязанный к текущему окну визуализации
    auto                         nodeLayers  = m_layerManager->getLayersForNode(baseNodeId);
    std::shared_ptr<Core::Layer> targetLayer = nullptr;
    for (const auto& layer : nodeLayers) {
        if (layer->view.lock() == vtkView) {
            targetLayer = layer;
            break;
        }
    }

    if (!targetLayer) {
        qCCritical(LogCore) << "Cannot start export: target layer not found for node" << baseNodeId;
        emit exportFinished(false);
        return;
    }

    // 2. Создаем VTK-экспортер видео
    auto exporter = std::make_shared<QSpace::Visualize::VideoExporter>(
        dynamic_cast<Visualize::VtkView*>(vtkView.get()));

    if (!exporter->startExport(outputPath, fps)) {
        emit exportFinished(false);
        return;
    }

    // 3. Конструируем асинхронный менеджер экспорта пакета кадров
    m_videoExportManager = std::make_shared<Core::VideoExportManager>(m_dataManager,
                                                                      targetLayer->renderEngine,
                                                                      exporter,
                                                                      this);

    // 4. Настраиваем трансляцию прогресса в UI
    connect(m_videoExportManager.get(),
            &Core::VideoExportManager::progressUpdated,
            this,
            &VideoController::exportProgressUpdated);

    connect(m_videoExportManager.get(),
            &Core::VideoExportManager::exportFinished,
            this,
            [this](bool success) {
                emit exportFinished(success);
                // Отложенное безопасное саморазрушение менеджера после завершения работы
                QTimer::singleShot(0, this, [this]() { m_videoExportManager.reset(); });
            });

    // 5. Генерируем схему парсинга по первому кадру и запускаем процесс
    QSpace::IO::FileFormat        format = QSpace::IO::Utils::getFormat(files.first());
    QSpace::Visualize::EntityType type =
        QSpace::IO::Utils::getEntityType(QFileInfo(files.first()).fileName());
    QSpace::IO::ReadScheme scheme = QSpace::IO::SchemeFactory::createSheme_v2(type, format);

    m_videoExportManager->start(files, scheme, format, stride);
}

void VideoController::cancelVideoExport() {
    if (m_videoExportManager) {
        m_videoExportManager->cancel();
    }
}

} // namespace QSpace::Core::Controllers