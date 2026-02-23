#pragma once
#include "Core/DataManager/DataManager.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/PipelineManager/PipelineManager.h"
#include "Core/TaskManager/TaskManager.h"
#include "Core/ViewManager/ViewManager.h"
#include "Structures/CoreStructures.h"
#include "Structures/IOStructures.h"
#include "Visualize/Renderer.h"
#include <QObject>
#include <functional>
#include <memory>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>

namespace QSpace::Core {
class AppCore : public QObject {
    Q_OBJECT
  public:
    explicit AppCore(QObject* parent = nullptr);
    ~AppCore() = default;

    void         initialize();
    void         setGroupingEnabled(bool enabled);
    void         updateNodeSettings(const QUuid& id, std::function<void(VisualSettings&)> modifer);
    DataManager* getDataManager() const {
        return m_dataManager.get();
    }
    ObjectRegistry* getObjectRegistry() const {
        return m_objectRegistry.get();
    }
    ViewManager* getViewManager() const {
        return m_viewManager.get();
    }
    LayerManager* getLayerManager() const {
        return m_layerManager.get();
    }
  private slots:
    void onFileReady(IO::ReadResult result);

  private:
    std::unique_ptr<TaskManager>     m_taskManager;
    std::unique_ptr<ObjectRegistry>  m_objectRegistry;
    std::unique_ptr<DataManager>     m_dataManager;
    std::unique_ptr<ViewManager>     m_viewManager;
    std::unique_ptr<PipelineManager> m_pipelineManager;
    std::unique_ptr<LayerManager>    m_layerManager;

    bool                           m_autoGrouping = true;
    QString                        extractGroupName(const QString& filename);
    std::shared_ptr<DataContainer> findOrCreateContainer(const QString& groupName);
};
} // namespace QSpace::Core