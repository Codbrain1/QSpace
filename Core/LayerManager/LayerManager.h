#pragma once
#include "Common/Interfaces/IRenderLayer.h"
#include "Common/Interfaces/IView.h"
#include "Common/Structures/CoreStructures.h"
#include <QList>
#include <QMap>
#include <QObject>
#include <QUuid>
#include <quuid.h>
#include <memory>

namespace QSpace::Core {


class LayerManager : public QObject {
    Q_OBJECT
  public:
    explicit LayerManager(QObject* parent = nullptr);
    ~LayerManager();

    QUuid createLayer(std::shared_ptr<DataNode> node, std::shared_ptr<Visualize::IView> view);
    void  removeLayer(const QUuid& layerId);
    void  removeAllLayersForNode(const QUuid& nodeId);

    // Метод для очистки всех слоев конкретного окна (вызывается при закрытии окна)
    void removeAllLayersForView(Visualize::IView* view);

    void populateNewView(std::shared_ptr<Visualize::IView>       newView,
                         const QList<std::shared_ptr<DataNode>>& allNodes);

    std::shared_ptr<Layer>        getLayer(const QUuid& layerId) const;
    QList<std::shared_ptr<Layer>> getLayersForNode(const QUuid& nodeId) const;

    void updateNodeMasterSettings(const QUuid& nodeId);

    void createLayersForContainer(std::shared_ptr<Snapshot>         container,
                                  std::shared_ptr<Visualize::IView> view);
    void setSnapshotVisibility(std::shared_ptr<Snapshot> container, bool visible);

    void setNodeVisibility(const QUuid& nodeId, bool visible);

    QList<std::shared_ptr<Layer>> getAllLayers() const {
        return m_layers.values();
    }

  signals:
    void layerCreated(const QUuid& layerId);
    void layerRemoved(const QUuid& layerId);

  private:
    QMap<QUuid, std::shared_ptr<Layer>> m_layers;
    QMap<QUuid, QList<QUuid>>           m_nodeToLayers;

    // Используем сырой указатель как ключ для группировки.
    // Мы не будем его разыменовывать без проверки через weak_ptr в самом Layer.
    QMap<Visualize::IView*, QList<QUuid>> m_viewToLayers;
};

} // namespace QSpace::Core