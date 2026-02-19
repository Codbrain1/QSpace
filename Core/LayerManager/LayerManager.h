#pragma once
#include "Common/Structures/CoreStructures.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/ViewManager/ViewManager.h"
#include "Renderer/Renderer.h"
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
    void createLayer(std::shared_ptr<DataNode> node, Visualize::Renderer* Renderer);
    void removeLayer(const QUuid& nodeId);
    void updateSettings(const QUuid& nodeId);
    ~LayerManager();

  signals:
    void layerCreated(const QUuid& nodeId);
    void layerRemoved(const QUuid& nodeId);

  private:
    // Структура хранения:
    // NodeID -> { Renderer* -> Layer }
    // Т.е. для одного набора данных у нас может быть много слоев (по одному на каждое окно)
    QMap<QUuid, QMap<Visualize::Renderer*, std::shared_ptr<Visualize::IRenderLayer>>> m_layers;
};
} // namespace QSpace::Core