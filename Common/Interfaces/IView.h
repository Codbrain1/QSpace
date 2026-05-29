#pragma once
#include "Common/Enums/RenderEnums.h"
#include <QObject>
#include <QUuid>
#include <QtWidgets/qwidget.h>
#include <qcontainerfwd.h>

namespace QSpace::Visualize
{

class IView : public QObject
{
  Q_OBJECT
public:
  explicit IView(QObject *parent = nullptr);
  virtual ~IView() = default;

  virtual QWidget *getWidget() = 0; // встраивается в QDockWidget или QMainWindow
  // Общий контракт перерисовки
  virtual void render() = 0;
  virtual void renderForce() = 0;
  virtual void setBackgroundColor(double r, double g, double b) = 0;

  // Виртуальные методы для 3D.
  // Для 2D-окон их можно будет оставить пустыми (default) или переопределить при необходимости.
  virtual void setAxesVisible(bool /*  visible */) = 0;
  virtual void setGridVisible(bool /* visible */) = 0;
  virtual void resetCamera() = 0;

  inline QString getViewName() const
  {
    return m_viewName;
  }
  inline void setViewName(const QString &name)
  {
    m_viewName = name;
  }
  inline ViewType getViewType() const
  {
    return m_viewType;
  }

signals:
  void updateRequested();

private:
  QString m_viewName;
  QSpace::Visualize::ViewType m_viewType;
};
class IView3D : public IView
{
  Q_OBJECT
public:
  explicit IView3D(QObject *parent = nullptr);
  virtual ~IView3D() = default;

  virtual void setCameraView(CameraViewType view) = 0;
  virtual void setGlobalExposure(double exposure) = 0;
};
class IView2D : public IView
{
  Q_OBJECT
public:
  explicit IView2D(QObject *parent = nullptr);
  virtual ~IView2D() = default;

  virtual void clearPlot() = 0;
  virtual void setXRange(double xmin, double xmax) = 0;
  virtual void setYRange(double ymin, double ymax) = 0;
};
} // namespace QSpace::Visualize