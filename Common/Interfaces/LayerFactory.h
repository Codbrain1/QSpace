#pragma once
#include "IRenderLayer.h"
#include "Structures/CoreStructures.h"
#include <memory>

namespace QSpace::Visualize
{
class LayerFactory
{
public:
  static std::shared_ptr<IRenderLayer> createLayer(const std::shared_ptr<Core::DataNode> node);
};
} // namespace QSpace::Visualize