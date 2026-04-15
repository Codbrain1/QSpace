#pragma once
#include "Common/Structures/CoreStructures.h"
#include <QWidget>
#include <memory>
#include <vtkProp.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>

namespace QSpace::Visualize
{

// Базовый интерфейс для любой отрисовки
class IRenderLayer
{
public:
  virtual ~IRenderLayer() = default;
  virtual void update() = 0;
  virtual void swapData(std::shared_ptr<QSpace::Core::DataNode> node) = 0;
  virtual void setVisible(bool visible) = 0;
  virtual bool isVisible() const = 0;
};

// Интерфейс исключительно для VTK
class IVtkRenderLayer : public IRenderLayer
{
public:
  virtual vtkSmartPointer<vtkProp> getVtkProp() = 0;
  virtual vtkSmartPointer<vtkScalarBarActor> getScalarBar() const = 0;
  virtual void attachInteractor(vtkRenderWindowInteractor *interactor) = 0;
  virtual void detachInteractor() = 0;
};

// Задел для 2D виджетов (QtCharts, QCustomPlot и т.д.)
class IWidgetRenderLayer : public IRenderLayer
{
public:
  virtual QWidget *getWidget() = 0;
};

} // namespace QSpace::Visualize