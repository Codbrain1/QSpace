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
    void handleRenderUpdate(); // запрос на обновление сцены
    void resetCamera();
    void handleCameraViewChange(QAction* action); // TODO
    void handleBackgroundChange(QAction* action);
    void axesVisibleToggled(bool visible);
    void gridVisibleToggled(bool visible);
    void handleVideoExport();
    void handleProjectSave();
    void handleProjectOpen();
    void handleSavePathSelection();
    void handleSessionStateChange(const QSpace::Session::CurrentSession& session);
    void handleExportFinished(bool success);
    void handleLayerSelectionChange(const QList<QUuid>& ids);
    void handleViewCreated(const QUuid& viewId, Visualize::ViewType type);
    void handleViewRemoved(const QUuid& viewId); // TODO: реализовать удаление доков для удаленных окон

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