#pragma once
#include "Common/Structures/CoreStructures.h"
#include <QObject>
#include <memory>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>

namespace QSpace::Core {
class ObjectRegistry : public QObject {
    Q_OBJECT
  public:
    explicit ObjectRegistry(QObject* parent = nullptr);

    // регистрация одиночного объекта
    void                             registerNode(std::shared_ptr<DataNode> node);
    void                             registerContainer(std::shared_ptr<DataContainer> container);
    std::shared_ptr<DataNode>        getNode(const QUuid& id) const;
    std::shared_ptr<DataContainer>   getContainer(const QUuid& id) const;
    void                             removeObject(const QUuid& id);
    std::shared_ptr<DataContainer>   findContainerByName(const QString& name) const;
    QList<std::shared_ptr<DataNode>> getAllNodes() {
        return m_nodes.values();
    }
    QList<std::shared_ptr<DataContainer>> getAllContainers() {
        return m_containers.values();
    }
    void clear() {
        for (const auto& node : m_nodes) {
            removeObject(node->id);
        }
        for (const auto& container : m_containers) {
            removeObject(container->id);
        }
    }
  signals:
    void nodeAdded(std::shared_ptr<QSpace::Core::DataNode> node);
    void containerAdded(std::shared_ptr<DataContainer> container);
    void objectRemoved(const QUuid& id);
    void cleared();

  private:
    QMap<QUuid, std::shared_ptr<DataNode>>      m_nodes;
    QMap<QUuid, std::shared_ptr<DataContainer>> m_containers;
};
} // namespace QSpace::Core