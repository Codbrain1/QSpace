#include "Common/Interfaces/LayerFactory.h"
#include "Common/Structures/CoreStructures.h"
#include "RenderLayerSettings/ParticleLayer.h"
#include <memory>
namespace QSpace::Visualize {
std::shared_ptr<IRenderLayer> LayerFactory::createLayer(const std::shared_ptr<Core::DataNode> node) {
    // TODO: реализовать другие классы для слоев и добавить их в фабрику слоев
    return std::make_shared<ParticleLayer>(node);
}
} // namespace QSpace::Visualize