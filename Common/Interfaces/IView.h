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
  virtual QString getViewName() const = 0;
  // Общий контракт перерисовки
  virtual void render() = 0;
  virtual void renderForce() = 0;
  virtual void setBackgroundColor(double r, double g, double b) = 0;

  // Виртуальные методы для 3D.
  // Для 2D-окон их можно будет оставить пустыми (default) или переопределить при необходимости.
  virtual void setAxesVisible(bool /*  visible */) {};
  virtual void setGridVisible(bool /* visible */) {};
  virtual void setCameraView(CameraViewType /* view */){}; // TODO: перечисление CameraViewType не подходит для 2d
  virtual void resetCamera() {};
  virtual void setGlobalExposure(double /* exposure */) {}; // установка глобальной прозрачности

signals:
  void updateRequested();
};

} // namespace QSpace::Visualize