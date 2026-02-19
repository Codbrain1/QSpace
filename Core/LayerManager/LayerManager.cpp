#include "LayerManager.h"
#include "Common/Logger/Logger.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/ViewManager/ViewManager.h"
#include "Enums/CoreEnums.h"
#include "Interfaces/LayerFactory.h"
#include "Renderer/ParticleLayer/ParticleLayer.h"
#include <memory>
#include <qobject.h>
#include <quuid.h>

namespace QSpace::Core {
LayerManager::LayerManager(QObject* parent) : QObject(parent) {
}

void LayerManager::createLayer(std::shared_ptr<DataNode> node, Visualize::Renderer* renderer) {
    if (!node || !renderer) {
        qCWarning(LogCore) << "LayerManager::createLayer - Invalid node or renderer";
        return;
    }

    qCInfo(LogCore) << "LayerManager::createLayer - Creating layer for node:" << node->label;

    auto layer = Visualize::LayerFactory::createLayer(node);
    // Фабрика слоев: выбираем реализацию в зависимости от типа данных
    if (layer) {
        qCInfo(LogCore) << "LayerManager::createLayer - Layer created, calling update()";
        layer->update(); // Применяем дефолтные настройки
        auto prop = layer->getVtkProp();
        if (prop) {
            renderer->addProp(prop); // Добавляем на сцену
            qCInfo(LogCore) << "LayerManager::createLayer - VtkProp added to renderer";
        }
        auto scalarBar = layer->getScalarBar();
        if (scalarBar) {
            renderer->addScalarBar(scalarBar);
        }
        m_layers[node->id][renderer] = layer; // Сохраняем
        emit layerCreated(node->id);
        qCInfo(LogCore) << "LayerManager::createLayer - Layer added to renderer";
    } else {
        qCWarning(LogCore) << "LayerManager::createLayer - Failed to create layer for node:" << node->label;
    }
}
void LayerManager::removeLayer(const QUuid& nodeId) {
    if (!m_layers.contains(nodeId)) {
        qCWarning(LogCore) << "LayerManager::removeLayer - Layer not found for node id:" << nodeId;
        return;
    }

    auto& renderersMap = m_layers[nodeId];

    // Удаляем пропы из всех рендереров
    for (auto it = renderersMap.begin(); it != renderersMap.end(); ++it) {
        Visualize::Renderer* renderer = it.key();
        auto                 layer    = it.value();
        if (renderer && layer) {
            renderer->removeProp(layer->getVtkProp());
            auto scalarBar = layer->getScalarBar();
            if (scalarBar)
                renderer->removeScalarBar(scalarBar);
        }
    }
    m_layers.remove(nodeId);
    emit layerRemoved(nodeId);
    qCInfo(LogCore) << "LayerManager::removeLayer - Layer removed";
}
void LayerManager::updateSettings(const QUuid& nodeId) {
    // 1. Находим все слои для этого узла (во всех окнах)
    if (!m_layers.contains(nodeId)) {
        qCWarning(LogCore) << "LayerManager::updateSettings - Layer not found for node id:" << nodeId;
        return;
    }

    auto& rendererMap = m_layers[nodeId];
    for (auto it = rendererMap.begin(); it != rendererMap.end(); ++it) {
        auto layer    = it.value();
        auto renderer = it.key();

        // 2. Просим слой обновиться (он сам возьмет данные из node->settings)
        layer->update();
        // 3. Перерисовываем конкретное окно
        renderer->render();
    }
    qCInfo(LogCore) << "LayerManager::updateSettings - Settings updated for node:" << nodeId;
}
LayerManager::~LayerManager() = default;
} // namespace QSpace::Core