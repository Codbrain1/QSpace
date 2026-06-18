#pragma once
#include "Common/Enums/RenderEnums.h"
#include "Interfaces/IView.h"
#include <QObject>
#include <quuid.h>

namespace QSpace::Core {
class ViewManager;
class LayerManager;
class ObjectRegistry;
} // namespace QSpace::Core

namespace QSpace::Core::Controllers {

// ---------------------------------------------------------
// @SECTION: обработка окон в приложении
// ---------------------------------------------------------
class ViewController : public QObject {
    Q_OBJECT
  public:
    explicit ViewController(Core::ViewManager*    viewManager,
                            Core::LayerManager*   layerManager,
                            Core::ObjectRegistry* objectRegistry,
                            QObject*              parent = nullptr);

    void initialize();

    QUuid             createView(Visualize::ViewType type = Visualize::ViewType::VTK_3D);
    void              removeView(const QUuid& viewId);
    Visualize::IView* getView(const QUuid& viewId);

    void resetCameraInAllViews();
    // void setCameraViewInAllViews(Visualize::CameraViewType viewType);
    void setBackgroundColorInAllViews(float r, float g, float b);
    void setAxesVisibleInAllViews(bool visible);
    void setGridVisibleInAllViews(bool visible);
    // void setGlobalExposureAllViews(double exposure);
    void resetCameraInView(const QUuid& viewId);
    // void setCameraViewInView(const QUuid& viewId, Visualize::CameraViewType viewType);
    void setBackgroundColorInView(const QUuid& viewId, float r, float g, float b);
    void setAxesVisibleInView(const QUuid& viewId, bool visible);
    void setGridVisibleInView(const QUuid& viewId, bool visible);

    // void  setGlobalExposureView(const QUuid& viewId, double exposure);
    Core::ViewManager* getViewManager() {
        return m_viewManager;
    };

  signals:
    void viewCreated(const QUuid& viewId, Visualize::ViewType type);
    void viewRemoved(const QUuid& viewId);
    void sceneUpdateRequested();

  private:
    Core::ViewManager*    m_viewManager;
    Core::LayerManager*   m_layerManager;
    Core::ObjectRegistry* m_objectRegistry;
};

} // namespace QSpace::Core::Controllers