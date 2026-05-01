#include "LayerManager.h"
#include "Common/Logger/Logger.h"
#include "Interfaces/IRenderLayer.h"
#include "Interfaces/IView.h"
#include "Interfaces/LayerFactory.h"
#include "Visualize/RenderLayerSettings/ParticleLayer.h"
#include <qloggingcategory.h>
#include <qobject.h>
#include <quuid.h>
#include "Enums/CoreEnums.h"
#include <memory>


namespace QSpace::Core {
LayerManager::LayerManager(QObject* parent) : QObject(parent) {
}

void LayerManager::createLayer(std::shared_ptr<DataNode> node, Visualize::IView* view) {
    if (!node || !view || !node->data) {
        qCWarning(LogCore) << "LayerManager::createLayer - Invalid node or renderer";
        return;
    }

    if (m_layers.contains(node->id) && m_layers[node->id].contains(view)) {
        qWarning() << "LayerManager::createLayer - Layer already exists for node:" << node->id;
        // Опционально: вызвать обновление существующего слоя вместо создания нового
        m_layers[node->id][view]->update();
        return;
    }
    qCDebug(LogCore) << "LayerManager::createLayer - Creating layer for node:" << node->label;

    auto layer = Visualize::LayerFactory::createLayer(node);
    if (!layer) {
        qCWarning(LogCore) << "LayerManager::createLayer - Failed to create layer for node:"
                           << node->label;
        return;
    }
    // Фабрика слоев: выбираем реализацию в зависимости от типа данных
    qCDebug(LogCore) << "LayerManager::createLayer - Layer created, calling update()";

    if (auto vtkView = qobject_cast<QSpace::Visualize::VtkView*>(view)) {
        if (auto layer3D = std::dynamic_pointer_cast<QSpace::Visualize::IVtkRenderLayer>(layer)) {
            vtkView->addProp(layer3D->getVtkProp());
            qCDebug(LogCore) << "LayerManager::createLayer - VtkProp added to renderer";
            if (auto interactor = vtkView->getInteractor()) {
                layer3D->attachInteractor(interactor);
            }
            connect(vtkView,
                    &QSpace::Visualize::VtkView::backgroundColorChanged,
                    this,
                    [layer3D](double contrast) { layer3D->updateColorsForContrast(contrast); });
        }
    }
    // TODO: раскоментировать когда будет добавлена поддержка 2d графиков
    //   else if (auto widgetView = qobject_cast<Visualize::IWidgetView*>(view)) {
    //      if (auto widgetLayer = std::dynamic_pointer_cast<Visualize::IWidgetRenderLayer>(layer))
    //      {
    //          // Окно 2D принимает виджет слоя для отображения
    //          widgetView->setWidget(widgetLayer->getWidget());
    //      }
    //  }
    //  TODO: убрать метод если он не нужен
    layer->update(); // Применяем дефолтные настройки

    m_layers[node->id][view] = layer; // Сохраняем

    emit layerCreated(node->id);
    qCDebug(LogCore) << "LayerManager::createLayer - Layer added to renderer";
}

void LayerManager::removeLayer(const QUuid& nodeId) {
    if (!m_layers.contains(nodeId)) {
        qCWarning(LogCore) << "LayerManager::removeLayer - Layer not found for node id:" << nodeId;
        return;
    }

    auto& renderersMap = m_layers[nodeId];

    // Удаляем пропы из всех рендереров
    for (auto it = renderersMap.begin(); it != renderersMap.end(); ++it) {
        Visualize::IView* view  = it.key();
        auto              layer = it.value();
        if (auto vtkView = qobject_cast<Visualize::VtkView*>(view)) {
            if (auto vtkLayer = std::dynamic_pointer_cast<Visualize::IVtkRenderLayer>(layer)) {
                vtkLayer->detachInteractor();
                vtkView->removeProp(vtkLayer->getVtkProp());
            }
        }
        // TODO: раскоментировать когда будет добавлена поддержка 2d графиков
        // else if (auto widgetView = qobject_cast<Visualize::IWidgetView*>(view)) {
        //     if (auto widgetLayer =
        //     std::dynamic_pointer_cast<Visualize::IWidgetRenderLayer>(layer)) {
        //         // Если слой удаляется, забираем его виджет из окна
        //         if (widgetView->getWidget() == widgetLayer->getWidget()) {
        //             widgetView->setWidget(nullptr);
        //         }
        //     }
        // }
    }
    m_layers.remove(nodeId);
    emit layerRemoved(nodeId);
    qCDebug(LogCore) << "LayerManager::removeLayer - Layer removed";
}

std::shared_ptr<Visualize::IRenderLayer> LayerManager::getLayer(const QUuid&      id,
                                                                Visualize::IView* renderer) {
    if (!m_layers.contains(id)) {
        qCWarning(LogCore) << "LayerManager::getLayer - Layer not found for node id:" << id;
        return nullptr;
    }
    auto& rendererMap = m_layers[id];
    if (!rendererMap.contains(renderer)) {
        qCWarning(LogCore) << "LayerManager::removeLayer - Layer not found for node Renderer";
        return nullptr;
    }
    return rendererMap[renderer];
}

void LayerManager::updateSettings(const QUuid& nodeId) {
    // 1. Находим все слои для этого узла (во всех окнах)
    if (!m_layers.contains(nodeId)) {
        qCWarning(LogCore) << "LayerManager::updateSettings - Layer not found for node id:"
                           << nodeId;
        return;
    }

    auto& rendererMap = m_layers[nodeId];
    for (auto it = rendererMap.begin(); it != rendererMap.end(); ++it) {
        auto layer = it.value();
        // 2. Просим слой обновиться (он сам возьмет данные из node->settings)
        layer->update();
    }
    qCDebug(LogCore) << "LayerManager::updateSettings - Settings updated for node:" << nodeId;
}

LayerManager::~LayerManager() = default;
} // namespace QSpace::Core