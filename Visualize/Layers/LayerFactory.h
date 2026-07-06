#pragma once
#include "Common/Interfaces/IRenderLayer.h"
#include <memory>

namespace QSpace::Visualize::Layers::LayerFactory {
std::shared_ptr<IRenderLayer> createLayerRenderer(const std::shared_ptr<Core::DataNode> node);

} // namespace QSpace::Visualize::Layers::LayerFactory