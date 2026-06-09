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

    // Регистрация объектов
    void registerNode(std::shared_ptr<DataNode> node);
    void registerNode(std::shared_ptr<DataNode> node, const QUuid& experimentId);
    void registerSnapshot(std::shared_ptr<Snapshot> container);
    void registerSnapshot(std::shared_ptr<Snapshot> container, const QUuid& experimentId);
    void registerExperiment(std::shared_ptr<Experiment> experiment);

    std::shared_ptr<DataNode>   getNode(const QUuid& id) const;
    std::shared_ptr<DataNode>   getOrLoadNodeData(const QUuid& id);
    std::shared_ptr<Snapshot>   getSnapshot(const QUuid& id) const;
    std::shared_ptr<Experiment> getExperiment(const QUuid& id) const;

    void                        removeObject(const QUuid& id);
    std::shared_ptr<Snapshot>   findSnapshotByName(const QString& name) const;
    std::shared_ptr<Experiment> findExperimentByName(const QString& name) const;

    void updateNodeData(const QUuid& id, vtkSmartPointer<vtkDataSet> dataSet, double timestamp);

    QList<std::shared_ptr<DataNode>> getAllNodes() {
        return m_nodes.values();
    }

    QList<std::shared_ptr<Snapshot>> getAllSnapshots() {
        return m_snapshots.values();
    }

    QList<std::shared_ptr<Experiment>> getAllExperiments() {
        return m_experiments.values();
    }

    void clear();
  signals:
    void nodeAdded(std::shared_ptr<QSpace::Core::DataNode> node);
    void snapshotAdded(std::shared_ptr<QSpace::Core::Snapshot> container);
    void experimentAdded(std::shared_ptr<QSpace::Core::Experiment> experiment);
    void objectRemoved(const QUuid& id);
    void cleared();

    // вызывается, когда данные ноды загрузились в ОЗУ или были выгружены кэшем
    void nodeDataUpdated(const QUuid& id);
    void
    dataLoadRequested(const QUuid& id, const QString& path, const QSpace::IO::ReadScheme& scheme);

  private:
    size_t                                  m_cacheCapacity = 10;
    std::list<QUuid>                        m_lruList;
    QMap<QUuid, std::list<QUuid>::iterator> m_lruMap;

    QMap<QUuid, std::shared_ptr<DataNode>>   m_nodes;
    QMap<QUuid, std::shared_ptr<Snapshot>>   m_snapshots;
    QMap<QUuid, std::shared_ptr<Experiment>> m_experiments;
    void                                     touchNodeInMemory(const QUuid& id);
};
} // namespace QSpace::Core