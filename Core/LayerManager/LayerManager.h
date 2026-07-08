#pragma once
#include "Common/Structures/ObjectRegistryStructures.h"
#include "Visualize/Layers/Layer.h"
#include "Visualize/Views/AbstractView.h"
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

    QUuid createLayer(std::shared_ptr<DataNode>                       node,
                      std::shared_ptr<Visualize::Views::AbstractView> view);
    void  removeLayer(const QUuid& layerId);
    void  removeAllLayersForNode(const QUuid& nodeId);

    // Метод для очистки всех слоев конкретного окна (вызывается при закрытии окна)
    void removeAllLayersForView(Visualize::Views::AbstractView* view);

    void populateNewView(std::shared_ptr<Visualize::Views::AbstractView> newView,
                         const QList<std::shared_ptr<DataNode>>&         allNodes);

    std::shared_ptr<Visualize::Layers::Layer>        getLayer(const QUuid& layerId) const;
    QList<std::shared_ptr<Visualize::Layers::Layer>> getLayersForNode(const QUuid& nodeId) const;

    void updateNodeMasterSettings(const QUuid& nodeId);

    void createLayersForContainer(std::shared_ptr<Snapshot>                       container,
                                  std::shared_ptr<Visualize::Views::AbstractView> view);
    void setSnapshotVisibility(std::shared_ptr<Snapshot> container, bool visible);

    void setNodeVisibility(const QUuid& nodeId, bool visible);

    QList<std::shared_ptr<Visualize::Layers::Layer>> getAllLayers() const {
        return m_layers.values();
    }

  signals:
    void layerCreated(const QUuid& layerId);
    void layerRemoved(const QUuid& layerId);

  private:
    QMap<QUuid, std::shared_ptr<Visualize::Layers::Layer>> m_layers;
    QMap<QUuid, QList<QUuid>>                              m_nodeToLayers;

    // Используем сырой указатель как ключ для группировки.
    // Мы не будем его разыменовывать без проверки через weak_ptr в самом Layer.
    QMap<Visualize::Views::AbstractView*, QList<QUuid>> m_viewToLayers;
};

} // namespace QSpace::Core