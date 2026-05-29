#pragma once
#include "Common/Interfaces/IView.h"
#include "Core/AppCore/AppCore.h"
#include "LayerExplorerWidget.h"
#include "PropertyInspector.h"
#include <QMainWindow>
#include <QObject>
#include <QProgressDialog>
#include <memory>
#include <qaction.h>
#include <qdockwidget.h>
#include <qmainwindow.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>

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
    void on_action_ChangedViewClicked(QAction* action);
    void on_action_ChangedBackgroundClicked(QAction* action);
    void on_action_toggleAxesChanged(bool checked);
    void on_action_toggleGridChanged(bool checked);
    void on_action_exportVideoClicked();
    void on_action_saveProjectClicked();
    void on_action_openProjectClicked();
    void on_savePathFromUIRequested();
    void on_sessionStateChange(const QSpace::Session::CurrentSession& session);
    void on_exportFinished(bool success);
    void on_LayerSelectionChanged(const QList<QUuid>& ids);
    void on_viewCreated(const QUuid& viewId, Visualize::ViewType type);
    void on_viewRemoved(const QUuid& viewId); // TODO: реализовать удаление доков для удаленных окон

  private:
    QSpace::Core::AppCore*               m_app;
    Ui::MainWindow*                      ui;
    std::unique_ptr<QProgressDialog>     m_exportProgressDialog;
    std::unique_ptr<LayerExplorerWidget> m_layerExplorerWidget;
    std::unique_ptr<PropertyInspector>   m_propertyInspector;
    QMap<QUuid, QDockWidget*> m_viewDockWidgets; // для хранения соответствия между viewId и их доками
    void                      setupSlots();
};
} // namespace QSpace::UI