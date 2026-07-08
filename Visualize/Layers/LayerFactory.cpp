#include "LayerFactory.h"
#include "Common/Enums/LayerEnums.h"
#include "Implementations/Binning/BinningRendererLayer.h"
#include "Implementations/Particle/ParticleRendererLayer.h"
#include "Implementations/SPH/SPHRendererLayer.h"


namespace QSpace::Visualize::Layers::LayerFactory {

namespace {

// Создаёт настройки нужного подтипа и переносит в них общие поля из masterSettings
// узла (opacity/colorByField/colorMapId/range и т.д.) через toVariantMap/fromVariantMap.
// Так один и тот же DataNode можно рендерить любым из трёх способов, не привязывая
// тип "мастер-настроек" узла к конкретному рендереру заранее.
template <typename SettingsT>
std::shared_ptr<SettingsT>
createSettingsInheritingBase(const std::shared_ptr<Core::DataNode>& node) {
    auto settings = std::make_shared<SettingsT>();
    if (node && node->masterSettings)
        settings->fromVariantMap(node->masterSettings->toVariantMap());
    return settings;
}

} // namespace

std::shared_ptr<IRenderLayer> createLayerRenderer(const std::shared_ptr<Core::DataNode> node,
                                                  Layers::RenderLayerType               type) {
    if (!node) {
        qWarning() << "LayerFactory::createLayerRenderer: node is null";
        return nullptr;
    }

    switch (type) {
        case Layers::RenderLayerType::Particles: {
            auto layer = std::make_shared<ParticleRendererLayer>();
            layer->setData(node);
            layer->setSettings(createSettingsInheritingBase<ParticlePointsLayerSettings>(node));
            return layer;
        }
        case Layers::RenderLayerType::SPH: {
            auto layer = std::make_shared<SPHRendererLayer>();
            layer->setData(node);
            layer->setSettings(createSettingsInheritingBase<SPHPointsLayerSettings>(node));
            return layer;
        }
        case Layers::RenderLayerType::Binning: {
            auto layer = std::make_shared<BinningRendererLayer>();
            layer->setData(node);
            layer->setSettings(createSettingsInheritingBase<BinningPointsLayerSettings>(node));
            return layer;
        }
    }

    qWarning() << "LayerFactory::createLayerRenderer: unknown RenderLayerType";
    return nullptr;
}

} // namespace QSpace::Visualize::Layers::LayerFactory