#pragma once
#include "Common/Enums/RenderEnums.h"
#include "Common/Interfaces/IView.h"
#include <QMap>
#include <QObject>
#include <QVTKOpenGLNativeWidget.h>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>
#include <qwidget.h>
#include <vtkCubeAxesActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkProp.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>
#include <vtkSmartPointerBase.h>
#include <memory>

namespace QSpace::Visualize {
class VtkView : public QSpace::Visualize::IView3D {
    Q_OBJECT
  public:
    explicit VtkView(QObject* parent = nullptr);
    virtual ~VtkView() = default;

    QWidget* getWidget() override;
    void     render() override;
    void     renderForce() override;

    void setBackgroundColor(double r, double g, double b) override;
    void setAxesVisible(bool visible) override;
    void setGridVisible(bool visible) override;

    void resetCamera() override; // Сброс камеры по границам данных
    void setCameraView(CameraViewType view) override;
    void setGlobalExposure(double exposure) override;

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> getRenderWindow() const;
    void                                          setRenderWindow(vtkRenderWindow* renderWindow);
    vtkRenderWindowInteractor*                    getInteractor() const;

    // добавлени объекта на сцену
    void addProp(vtkSmartPointer<vtkProp> prop);
    void removeProp(vtkSmartPointer<vtkProp> prop);

    // общие настройки
    void setCenterOfRotation(double x, double y, double z); // Перенос "центра"
                                                            //  управление камерой
  signals:
    void updateRequested(); // TODO: убрать в будущем?
    void backgroundColorChanged(double contrast);

  private:
    std::unique_ptr<QVTKOpenGLNativeWidget>       m_vtkWidget;
    vtkSmartPointer<vtkRenderer>                  m_vtkRenderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkCubeAxesActor>             m_axesActor;
    vtkSmartPointer<vtkOrientationMarkerWidget>   m_orientationMarker;
    void                                          setupAxes();
    void                                          setupDepthPeeling(); // Важно для прозрачности!
};
} // namespace QSpace::Visualize