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

// Вспомогательная структура Layer остается в этом же файле или CoreStructures.h
// Я добавил метод инициализации, чтобы избежать проблем с порядком создания
struct Layer {
    QUuid                                    layerId;
    QUuid                                    dataNodeId;
    QString                                  name;
    std::weak_ptr<DataNode>                  dataNode;
    std::weak_ptr<Visualize::IView>          view;
    std::shared_ptr<VisualSettings>          settings;
    std::shared_ptr<Visualize::IRenderLayer> renderEngine;
    bool                                     isSyncedWithMaster = true;

    Layer(std::shared_ptr<DataNode> node, std::shared_ptr<Visualize::IView> targetView)
        : layerId(QUuid::createUuid()),
          dataNodeId(node->id),
          dataNode(node),
          view(targetView),
          settings(node->masterSettings) {
    }

    // Метод для связки после создания renderEngine
    void assignEngine(std::shared_ptr<Visualize::IRenderLayer> engine) {
        renderEngine = engine;
        if (renderEngine) {
            renderEngine->setSettings(settings);
            renderEngine->setData(dataNode);
        }
    }

    void setVisible(bool visible) {
        settings->isVisible = visible;
        renderEngine->setVisible(visible);
    }

    void update() {
        auto v = view.lock();
        auto d = dataNode.lock();
        if (v && d && renderEngine) {
            renderEngine->update();
        }
    }
};

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