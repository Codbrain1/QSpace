#pragma once
#include "Common/Enums/RenderEnums.h"
#include <QMap>
#include <QObject>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>
#include <vtkCubeAxesActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkProp.h>
#include <vtkRenderWindow.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>

namespace QSpace::Visualize {
class Renderer : public QObject {
    Q_OBJECT
  public:
    explicit Renderer(QObject* parent = nullptr);
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> getRenderWindow() const;
    // добавлени объекта на сцену
    void addProp(vtkSmartPointer<vtkProp> prop);
    void removeProp(vtkSmartPointer<vtkProp> prop);

    // управление легендой
    void addScalarBar(vtkSmartPointer<vtkScalarBarActor> bar);
    void removeScalarBar(vtkSmartPointer<vtkScalarBarActor> bar);

    // общие настройки
    void setBackgroundColor(double r, double g, double b);
    void setAxesVisible(bool visible);
    void setCameraView(CameraViewType view);
    void setCenterOfRotation(double x, double y, double z); // Перенос "центра"

    //  управление камерой
    void resetCamera(); // Сброс камеры по границам данных
    void render();
    void renderForce();
  signals:
    void updateRequested();

  private:
    vtkSmartPointer<vtkRenderer>                  m_vtkRenderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderWindow;
    vtkSmartPointer<vtkCubeAxesActor>             m_axesActor;

    void setupAxes();
    void setupDepthPeeling(); // Важно для прозрачности!
};
} // namespace QSpace::Visualize