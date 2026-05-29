#pragma once
#include "IRenderLayer.h"
#include "Structures/CoreStructures.h"
#include <memory>

namespace QSpace::Visualize
{
class LayerEngineFactory
{
public:
  static std::shared_ptr<IRenderLayer> createLayerEngine(const std::shared_ptr<Core::DataNode> node);
};
} // namespace QSpace::Visualize