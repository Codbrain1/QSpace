#pragma once
#include "Common/Structures/CoreStructures.h"
#include <QObject>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>
#include <memory>

namespace QSpace::Core {
class ObjectRegistry : public QObject {
    Q_OBJECT
  public:
    explicit ObjectRegistry(QObject* parent = nullptr);

    // регистрация одиночного объекта
    void registerNode(std::shared_ptr<DataNode> node);
    void registerNodeWithGrouping(std::shared_ptr<DataNode> node, const QUuid& experimentId);
    void registerContainer(std::shared_ptr<DataContainer> container);
    void registerExperiment(std::shared_ptr<Experiment> experiment);
    std::shared_ptr<DataNode>      getNode(const QUuid& id) const;
    std::shared_ptr<DataNode>      getOrLoadNodeData(const QUuid& id);
    std::shared_ptr<DataContainer> getContainer(const QUuid& id) const;
    std::shared_ptr<Experiment>    getExperiment(const QUuid& id) const;
    void                           removeObject(const QUuid& id);
    std::shared_ptr<DataContainer> findContainerByName(const QString& name) const;

    void                      updateNodeData(const QUuid& id, vtkSmartPointer<vtkDataSet> dataSet);
    std::shared_ptr<DataNode> getNodeAndTouch(const QUuid& id);

    QList<std::shared_ptr<DataNode>> getAllNodes() {
        return m_nodes.values();
    }

    QList<std::shared_ptr<DataContainer>> getAllContainers() {
        return m_containers.values();
    }

    QList<std::shared_ptr<Experiment>> getAllExperiments() {
        return m_experiments.values();
    }

    void clear() {
        m_nodes.clear();
        m_containers.clear();
        m_experiments.clear();
        m_lruList.clear();
        m_lruMap.clear();
        emit cleared();
    }

  signals:
    void nodeAdded(std::shared_ptr<QSpace::Core::DataNode> node);
    void containerAdded(std::shared_ptr<QSpace::Core::DataContainer> container);
    void experimentAdded(std::shared_ptr<QSpace::Core::Experiment> experiment);
    void objectRemoved(const QUuid& id);
    void cleared();

    // вызывается, когда данные ноды загрузились в ОЗУ или были выгружены кэшем
    void nodeDataUpdated(const QUuid& id);

  private:
    size_t           m_cacheCapacity = 10;
    std::list<QUuid> m_lruList; // Хранит ID только тех нод, у которых данные СЕЙЧАС в ОЗУ
    QMap<QUuid, std::list<QUuid>::iterator>     m_lruMap;
    QMap<QUuid, std::shared_ptr<DataNode>>      m_nodes;
    QMap<QUuid, std::shared_ptr<DataContainer>> m_containers;
    QMap<QUuid, std::shared_ptr<Experiment>>    m_experiments;
    void                                        touchNodeInMemory(const QUuid& id);
};
} // namespace QSpace::Core