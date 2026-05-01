#pragma once
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/ViewManager/ViewManager.h"
#include <QObject>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>
#include <vtkProp.h>
#include <vtkSmartPointer.h>
#include <memory>


namespace QSpace::Core {
class PipelineManager : public QObject {
    Q_OBJECT
  public:
    explicit PipelineManager(ObjectRegistry* registry,
                             ViewManager*    viewManager,
                             LayerManager*   LayerManager,
                             QObject*        parent = nullptr);
  public slots:
    void onNodeAdded(std::shared_ptr<DataNode> node);
    void onObjectRemoved(const QUuid& id);
    void onViewCreated(const QUuid& viewId);

  private:
    ViewManager*    m_viewManager;
    LayerManager*   m_layerManager;
    ObjectRegistry* m_registry;
};
} // namespace QSpace::Core