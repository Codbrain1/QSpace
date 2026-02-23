#include "MainWindow.h"
#include "Common/Logger/Logger.h"
#include "Core/AppCore/AppCore.h"
#include "DataTreeController.h"
#include "Enums/CommonEnumsIO.h"
#include "Enums/RenderEnums.h"
#include "Interfaces/IOFactory.h"
#include "PropertyInspector.h"
#include "ui_newmainwindow.h"
#include <QMainWindow>
#include <QPushButton>
#include <QVTKOpenGLNativeWidget.h>
#include <memory>
#include <qaction.h>
#include <qcontainerfwd.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qmainwindow.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <vtkDataSetAttributes.h>
#include <vtkType.h>

namespace QSpace::UI {
MainWindow::MainWindow(Core::AppCore* app, QWidget* parent)
    : QMainWindow(parent), m_app(app), m_renderer(nullptr), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    // Получаем рендерер основного окна из ViewManager
    auto mainViewId = app->getViewManager()->getMainViewId();
    m_renderer      = app->getViewManager()->getView(mainViewId);

    if (m_renderer) {
        ui->vtkWidget->setRenderWindow(m_renderer->getRenderWindow());
        connect(m_renderer, &Visualize::Renderer::updateRequested, this, &MainWindow::on_render_update);

        // ---- Подключение кнопок QToolBar ----

        // сброс камеры
        connect(ui->action_reset_camera, &QAction::triggered, this, &MainWindow::on_action_resetCameraClicked);
        connect(ui->action_view_top, &QAction::triggered, this, &MainWindow::on_action_viewTopClicked);
        connect(ui->action_toggle_axes, &QAction::toggled, this, &MainWindow::on_action_toggleAxesChanged);
        connect(ui->action_toggle_grid, &QAction::toggled, this, &MainWindow::on_action_toggleGridChanged);
    }
    m_dataTreeController = std::make_unique<DataTreeController>(app, ui->tree_layers, app->getObjectRegistry());
    connect(ui->btn_add_data, &QPushButton::clicked, this, &MainWindow::on_btn_add_data);
    m_propertyInspector = std::make_unique<PropertyInspector>(app);
    ui->dock_properties->setWidget(m_propertyInspector.get());
    connect(m_dataTreeController.get(),
            &DataTreeController::selectionChanged,
            m_propertyInspector.get(),
            [inspector = m_propertyInspector.get()](const QList<QUuid>& ids) {
                if (ids.isEmpty()) {
                    inspector->setCurrentNode(QUuid());
                } else {
                    // Берем первый выбранный элемент и передаем в инспектор
                    inspector->setCurrentNode(ids.first());
                }
            });
}
MainWindow::~MainWindow() {
    delete ui;
}
void MainWindow::on_action_resetCameraClicked() {
    if (m_renderer)
        m_renderer->resetCamera();
}
void MainWindow::on_action_viewTopClicked() {
    if (m_renderer)
        m_renderer->setCameraView(QSpace::Visualize::CameraViewType::XY_Top);
}
void MainWindow::on_action_toggleAxesChanged(bool checked) {
    if (m_renderer)
        m_renderer->setAxesVisible(checked);
}
void MainWindow::on_action_toggleGridChanged(bool checked) {
    // TODO::добавить отображение сетки
}
// ------------------------- слоты обработка действий пользователя ------------------------------
void MainWindow::on_render_update() {
    qCDebug(LogSystem) << "MainWindow::on_render_update() called";
    if (ui->vtkWidget->isVisible() && m_renderer) {
        ui->vtkWidget->renderWindow()->Render(); // Явный рендер VTK
    }
}
void MainWindow::on_btn_add_data() {
    QStringList paths = QFileDialog::getOpenFileNames(this, "выберите файлы с данными", "", "Bin Files (*.bin)");
    if (paths.isEmpty())
        return;
    if (paths.size() == 1) {
        IO::FileFormat        format     = IO::Utils::getFormat(paths[0]);
        Visualize::EntityType entityType = IO::Utils::getEntityType(QFileInfo(paths[0]).fileName());
        IO::ReadScheme        scheme     = IO::ShemeFactory::createDefaultSheme(entityType, format);
        m_app->getDataManager()->importDataAsync(paths[0], scheme);
    }
}
void MainWindow::on_btn_remove_data() {
    auto items = ui->tree_layers->selectedItems();

    for (auto* item : items) {
        QString idStr = item->data(0, Qt::UserRole).toString();
        if (!idStr.isEmpty()) {
            m_app->getObjectRegistry()->removeObject(QUuid::fromString(idStr));
        }
    }
}
} // namespace QSpace::UI