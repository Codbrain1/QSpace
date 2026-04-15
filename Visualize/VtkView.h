#pragma once
#include "Common/Enums/RenderEnums.h"
#include "Common/Interfaces/IView.h"
#include <QMap>
#include <QObject>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>
#include <vtkCubeAxesActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkProp.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>
#include <vtkSmartPointerBase.h>

namespace QSpace::Visualize {
class VtkView : public QSpace::Visualize::IView {
    Q_OBJECT
  public:
    explicit VtkView(QObject* parent = nullptr);
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> getRenderWindow() const;
    void                                          setRenderWindow(vtkRenderWindow* renderWindow);
    vtkRenderWindowInteractor*                    getInteractor() const;
    // добавлени объекта на сцену
    void addProp(vtkSmartPointer<vtkProp> prop);
    void removeProp(vtkSmartPointer<vtkProp> prop);

    // общие настройки
    void setBackgroundColor(double r, double g, double b) override;
    void setAxesVisible(bool visible) override;
    void setGridVisible(bool visible) override;
    void setCameraView(CameraViewType view) override;
    void setCenterOfRotation(double x, double y, double z); // Перенос "центра"
    //  управление камерой
    void resetCamera() override; // Сброс камеры по границам данных
    void render() override;
    void renderForce() override;
    void setGlobalExposure(double exposure) override;
  signals:
    void updateRequested(); // TODO: убрать в будущем?

  private:
    vtkSmartPointer<vtkRenderer>                  m_vtkRenderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkCubeAxesActor>             m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget>   m_orientationMarker;
    void                                          setupAxes();
    void                                          setupDepthPeeling(); // Важно для прозрачности!
};
} // namespace QSpace::Visualize