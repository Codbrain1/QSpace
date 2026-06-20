#include "LayerManager.h"
#include "Common/Logger/Logger.h"
#include "Core/StyleManager/StyleManager.h"
#include "Interfaces/LayerFactory.h"
#include "Visualize/VtkView.h"

namespace QSpace::Core {

LayerManager::LayerManager(QObject* parent) : QObject(parent) {
}

LayerManager::~LayerManager() = default;

QUuid LayerManager::createLayer(std::shared_ptr<DataNode>         node,
                                std::shared_ptr<Visualize::IView> view) {
    if (!node || !view || !node->data) {
        qCWarning(LogCore) << "LayerManager::createLayer - Invalid node or view";
        return QUuid();
    }

    // 1. Создаем обертку
    auto layer = std::make_shared<Layer>(node, view);

    // 2. Создаем движок через фабрику
    auto renderEngine = Visualize::LayerEngineFactory::createLayerEngine(node);
    if (!renderEngine)
        return QUuid();

    // 3. Инициализируем движок данными и настройками
    layer->assignEngine(renderEngine);

    // 4. Привязываем к VTK
    if (auto vtkView = qobject_cast<QSpace::Visualize::VtkView*>(view.get())) {
        if (auto layer3D = std::dynamic_pointer_cast<QSpace::Visualize::IVtkRenderLayer>(
                layer->renderEngine)) {
            vtkView->addProp(layer3D->getVtkProp());
            if (auto interactor = vtkView->getInteractor()) {
                layer3D->attachInteractor(interactor);
            }
        }
    }

    layer->update();

    // 5. Регистрация в индексах
    m_layers.insert(layer->layerId, layer);
    m_nodeToLayers[node->id].append(layer->layerId);
    m_viewToLayers[view.get()].append(layer->layerId);

    emit layerCreated(layer->layerId);
    return layer->layerId;
}

void LayerManager::removeLayer(const QUuid& layerId) {
    if (!m_layers.contains(layerId))
        return;

    auto layer      = m_layers[layerId];
    auto rawViewPtr = layer->view.lock().get(); // Получаем адрес окна, если оно живо

    // Отвязываем от View (только если окно еще существует)
    if (rawViewPtr) {
        if (auto vtkView = qobject_cast<Visualize::VtkView*>(rawViewPtr)) {
            if (auto vtkLayer =
                    std::dynamic_pointer_cast<Visualize::IVtkRenderLayer>(layer->renderEngine)) {
                vtkLayer->detachInteractor();
                vtkView->removeProp(vtkLayer->getVtkProp());
            }
        }
        // Удаляем из индекса окна
        if (m_viewToLayers.contains(rawViewPtr)) {
            m_viewToLayers[rawViewPtr].removeOne(layerId);
            if (m_viewToLayers[rawViewPtr].isEmpty())
                m_viewToLayers.remove(rawViewPtr);
        }
    }

    // Удаляем из остальных индексов
    m_nodeToLayers[layer->dataNodeId].removeOne(layerId);
    m_layers.remove(layerId);

    emit layerRemoved(layerId);
}

void LayerManager::removeAllLayersForNode(const QUuid& nodeId) {
    // Используем временный список, так как removeLayer модифицирует m_nodeToLayers
    QList<QUuid> toRemove = m_nodeToLayers.value(nodeId);
    for (const auto& id : toRemove) {
        removeLayer(id);
    }
}

void LayerManager::removeAllLayersForView(Visualize::IView* view) {
    if (!m_viewToLayers.contains(view))
        return;

    QList<QUuid> toRemove = m_viewToLayers.value(view);
    for (const auto& id : toRemove) {
        removeLayer(id);
    }
}

void LayerManager::populateNewView(std::shared_ptr<Visualize::IView>       newView,
                                   const QList<std::shared_ptr<DataNode>>& allNodes) {
    for (const auto& node : allNodes) {
        createLayer(node, newView);
    }
}

std::shared_ptr<Layer> LayerManager::getLayer(const QUuid& layerId) const {
    return m_layers.value(layerId, nullptr);
}

QList<std::shared_ptr<Layer>> LayerManager::getLayersForNode(const QUuid& nodeId) const {
    QList<std::shared_ptr<Layer>> result;
    for (const auto& id : m_nodeToLayers.value(nodeId)) {
        if (m_layers.contains(id))
            result.append(m_layers[id]);
    }
    return result;
}

void LayerManager::updateNodeMasterSettings(const QUuid& nodeId) {
    auto layers = getLayersForNode(nodeId);
    for (auto& layer : layers) {
        if (layer->isSyncedWithMaster) {
            layer->update();
        }
    }
}

// 1. Создание слоев — оставляем как есть, это надежно
void LayerManager::createLayersForContainer(std::shared_ptr<Snapshot>         container,
                                            std::shared_ptr<Visualize::IView> view) {
    if (!container || !view)
        return;

    for (auto& node : container->components) {
        this->createLayer(node, view);
    }
}

// 2. Управление видимостью — добавляем флаг блокировки рендера
void LayerManager::setSnapshotVisibility(std::shared_ptr<Snapshot> container, bool visible) {
    if (!container)
        return;

    // Обновляем узлы текущего контейнера
    for (auto& node : container->components) {
        // Мы используем прямой доступ к ID, это быстро
        auto layerIds = m_nodeToLayers.value(node->id);
        for (const auto& id : layerIds) {
            if (auto layer = m_layers.value(id)) {
                layer->settings->isVisible = visible;
                // Вызываем обновление мапперов VTK, но БЕЗ немедленного рендера окна
                if (layer->renderEngine) {
                    layer->renderEngine->setVisible(visible);
                }
            }
        }
    }
}

void LayerManager::setNodeVisibility(const QUuid& nodeId, bool visible) {
    auto layers = getLayersForNode(nodeId);
    for (auto& layer : layers) {
        if (layer->isSyncedWithMaster) {
            layer->setVisible(visible);
        }
    }
}
} // namespace QSpace::Core