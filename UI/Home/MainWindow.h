#pragma once
#include "Core/AppCore/AppCore.h"
#include "DataTreeController.h"
#include "PropertyInspector.h"
#include "Renderer/Renderer.h"
#include <QMainWindow>
#include <QObject>
#include <memory>
#include <qmainwindow.h>
#include <qobject.h>
#include <qtmetamacros.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
} // namespace Ui
QT_END_NAMESPACE

namespace QSpace::UI {
class MainWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit MainWindow(Core::AppCore* app, QWidget* parent = nullptr);
    ~MainWindow();

  private slots:
    void on_btn_add_data();
    void on_btn_remove_data();
    void on_render_update();
    void on_action_resetCameraClicked();
    void on_action_viewTopClicked();
    void on_action_toggleAxesChanged(bool checked);
    void on_action_toggleGridChanged(bool checked);

  private:
    Core::AppCore*                      m_app;
    Visualize::Renderer*                m_renderer;
    Ui::MainWindow*                     ui;
    std::unique_ptr<DataTreeController> m_dataTreeController;
    std::unique_ptr<PropertyInspector>  m_propertyInspector;
};
} // namespace QSpace::UI