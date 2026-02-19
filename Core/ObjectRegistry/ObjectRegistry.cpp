#include "ObjectRegistry.h"
#include "Common/Logger/Logger.h"
#include <memory>
#include <qcontainerfwd.h>
#include <qloggingcategory.h>
#include <qobject.h>
#include <quuid.h>
namespace QSpace::Core {
ObjectRegistry::ObjectRegistry(QObject* parent) : QObject(parent) {
}
void ObjectRegistry::registerNode(std::shared_ptr<DataNode> node) {
    if (!node)
        return;
    if (m_nodes.contains(node->id))
        return;
    m_nodes.insert(node->id, node);
    emit nodeAdded(node);
    qCInfo(LogCore) << "Registry: Node registered " << node->label << " " << node->id;
}
void ObjectRegistry::registerContainer(std::shared_ptr<DataContainer> container) {
    if (!container)
        return;
    if (m_containers.contains(container->id))
        return;
    m_containers.insert(container->id, container);
    for (auto& comp : container->components) {
        registerNode(comp);
    }
    emit containerAdded(container);
    qCInfo(LogCore) << "Registry: Container registered " << container->name << " " << container->id;
}
std::shared_ptr<DataNode> ObjectRegistry::getNode(const QUuid& id) const {
    return m_nodes.value(id, nullptr);
}
std::shared_ptr<DataContainer> ObjectRegistry::getContainer(const QUuid& id) const {
    return m_containers.value(id, nullptr);
}
void ObjectRegistry::removeObject(const QUuid& id) {
    if (m_containers.contains(id)) {
        auto container = m_containers.take(id);
        qCInfo(LogCore) << "Registry: Removing container" << container->name;
        for (const auto& node : container->components) {
            if (m_nodes.contains(node->id)) {
                m_nodes.remove(node->id);
                emit objectRemoved(node->id);
            }
        }
        emit objectRemoved(id);
        return;
    }
    if (m_nodes.contains(id)) {
        for (auto& container : m_containers) {
            container->components.erase(
                std::remove_if(container->components.begin(),
                               container->components.end(),
                               [&id](const std::shared_ptr<DataNode>& n) { return n->id == id; }),
                container->components.end());
        }
        m_nodes.remove(id);
        emit objectRemoved(id);
        qCInfo(LogCore) << "Registry: Node removed" << id;
    }
}
std::shared_ptr<DataContainer> ObjectRegistry::findContainerByName(const QString& name) const {
    for (auto container : m_containers) {
        if (container->name == name) {
            return container;
        }
    }
    return nullptr;
}
} // namespace QSpace::Core