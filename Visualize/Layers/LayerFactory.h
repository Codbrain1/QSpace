#pragma once
#include "Common/Enums/LayerEnums.h"
#include "Common/Interfaces/IRenderLayer.h"
#include <memory>

namespace QSpace::Visualize::Layers::LayerFactory {
std::shared_ptr<IRenderLayer>
createLayerRenderer(const std::shared_ptr<Core::DataNode> node,
                    Layers::RenderLayerType type = Layers::RenderLayerType::Particles);
std::shared_ptr<IRenderLayer>
createLayerRenderer(Layers::RenderLayerType type = Layers::RenderLayerType::Particles);
} // namespace QSpace::Visualize::Layers::LayerFactory