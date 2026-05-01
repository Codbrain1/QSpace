#pragma once
#include "Common/Structures/CoreStructures.h"
#include "Interfaces/IView.h"
#include "Visualize/VtkView.h"
#include <Common/Interfaces/IRenderLayer.h>
#include <QMap>
#include <memory>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>

namespace QSpace::Core {
class LayerManager : public QObject {
    Q_OBJECT
  public:
    LayerManager(QObject* parent = nullptr);
    void createLayer(std::shared_ptr<DataNode> node, QSpace::Visualize::IView* view);
    void removeLayer(const QUuid& nodeId);
    std::shared_ptr<Visualize::IRenderLayer> getLayer(const QUuid& id, Visualize::IView* renderer);
    void                                     updateSettings(const QUuid& nodeId);

    ~LayerManager();

  signals:
    void layerCreated(const QUuid& nodeId);
    void layerRemoved(const QUuid& nodeId);

  private:
    // void recreateLayer(std::shared_ptr<DataNode> node, Visualize::IView* view);
    // Структура хранения:
    // NodeID -> { View* -> Layer }
    // Т.е. для одного набора данных у нас может быть много слоев (по одному на каждое окно)
    QMap<QUuid, QMap<QSpace::Visualize::IView*, std::shared_ptr<QSpace::Visualize::IRenderLayer>>> m_layers;
};
} // namespace QSpace::Core