#include "MainWindow.h"
#include "Common/Logger/Logger.h"
#include "Core/AppCore/AppCore.h"
#include "DataTreeController.h"
#include "Enums/CommonEnumsIO.h"
#include "Enums/RenderEnums.h"
#include "Interfaces/IOFactory.h"
#include "PropertyInspector.h"
#include "Structures/IOStructures.h"
#include "ui_newmainwindow.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QVTKOpenGLNativeWidget.h>
#include <cstddef>
#include <memory>
#include <qaction.h>
#include <qcontainerfwd.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlist.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qsharedpointer.h>
#include <qtoolbutton.h>
#include <vtkDataSetAttributes.h>
#include <vtkType.h>

namespace QSpace::UI {
MainWindow::MainWindow(Core::AppCore* app, QWidget* parent)
    : QMainWindow(parent), m_app(app), m_renderer(nullptr), ui(new Ui::MainWindow) {
    // инициализируем ui файл
    ui->setupUi(this);

    // Получаем рендерер основного окна из ViewManager
    auto mainViewId = app->getViewManager()->getMainViewId();
    m_renderer      = app->getViewManager()->getView(mainViewId);

    // инициализируем меню слоев DataTreeController
    //--------------------------------------------------
    m_dataTreeController = std::make_unique<DataTreeController>(app, ui->tree_layers, app->getObjectRegistry());

    // инициализируем меню настроек слоя PropertyInspector
    //--------------------------------------------------
    m_propertyInspector = std::make_unique<PropertyInspector>(app);
    ui->dock_properties->setWidget(m_propertyInspector.get());

    // --- ПОДГОТОВКА ДИАЛОГА ПРОГРЕССА ---
    //--------------------------------------------------

    m_exportProgressDialog = std::make_unique<QProgressDialog>("Рендеринг видео...", "Отмена", 0, 100, this);
    m_exportProgressDialog->setWindowTitle("Экспорт анимации");
    m_exportProgressDialog->setWindowModality(Qt::WindowModal); // Блокируем главное окно
    m_exportProgressDialog->setAutoClose(true);
    m_exportProgressDialog->setAutoReset(true);
    m_exportProgressDialog->reset(); // Скрываем по умолчанию

    // Привязываем значения перечисления к действиям через Data (удобно для обработки в одном слоте)
    ui->action_view_top->setData(static_cast<int>(Visualize::CameraViewType::XY_Top));
    ui->action_view_front->setData(static_cast<int>(Visualize::CameraViewType::XZ_Front));
    ui->action_view_right->setData(static_cast<int>(Visualize::CameraViewType::YZ_Right));
    ui->action_view_iso->setData(static_cast<int>(Visualize::CameraViewType::Iso));

    // 2. Превращаем экшен "action_view_top" в выпадающее меню камеры
    if (auto* btn = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(ui->action_view_top))) {
        btn->setMenu(ui->menu_camera); // Берем уже готовое меню из UI
        btn->setPopupMode(QToolButton::InstantPopup);
    }
    if (auto* btn = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(ui->action_bg_settings))) {
        btn->setMenu(ui->menu_bg); // Берем уже готовое меню из UI
        btn->setPopupMode(QToolButton::InstantPopup);
    }
    QToolButton* viewButton = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(ui->action_view_top));
    if (viewButton) {
        // viewButton->setMenu(viewMenu);
        viewButton->setPopupMode(QToolButton::InstantPopup); // Меню открывается сразу при клике
    }

    // подключаем слоты
    setupSlots();
}
void MainWindow::setupSlots() {
    if (m_renderer) {
        ui->vtkWidget->setRenderWindow(m_renderer->getRenderWindow());
        connect(m_renderer, &Visualize::Renderer::updateRequested, this, &MainWindow::on_render_update);

        // ---- Подключение кнопок QToolBar ----
        // работа со сценой
        connect(ui->action_reset_camera, &QAction::triggered, this, &MainWindow::on_action_resetCameraClicked);
        connect(ui->action_toggle_axes, &QAction::toggled, this, &MainWindow::on_action_toggleAxesChanged);
        connect(ui->action_toggle_grid, &QAction::toggled, this, &MainWindow::on_action_toggleGridChanged);

        // запуск рендеринга видео
        QAction* actionExport = ui->mainToolBar->addAction(QIcon::fromTheme("video-x-generic"), "Экспорт видео");
        connect(actionExport, &QAction::triggered, this, &MainWindow::on_action_exportVideoClicked);
    }

    // --- СОЗДАНИЕ АНИМАЦИИ ---
    connect(m_exportProgressDialog.get(), &QProgressDialog::canceled, m_app, &Core::AppCore::cancelVideoExport);
    connect(m_app, &Core::AppCore::exportProgressUpdated, this, [this](int current, int total) {
        m_exportProgressDialog->setMaximum(total);
        m_exportProgressDialog->setValue(current);
    });
    connect(m_app, &Core::AppCore::exportFinished, this, [this](bool success) {
        m_exportProgressDialog->reset(); // Прячем окно
        if (success) {
            QMessageBox::information(this, "Готово", "Видео успешно сохранено!");
        } else {
            QMessageBox::warning(this, "Отмена", "Экспорт видео прерван или завершен с ошибкой.");
        }
    });

    // Добавление новых данных
    connect(ui->btn_add_data, &QPushButton::clicked, this, &MainWindow::on_btn_add_data);
    connect(ui->btn_remove_data, &QPushButton::clicked, this, &MainWindow::on_btn_remove_data);
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

    connect(ui->menu_camera, &QMenu::triggered, this, &MainWindow::on_action_ChangedViewClicked);
    connect(ui->menu_bg, &QMenu::triggered, this, &MainWindow::on_action_ChangedBackgroundClicked);
}
// РЕАЛИЗАЦИЯ СЛОТА
void MainWindow::on_action_exportVideoClicked() {
    // 1. Проверяем, выбран ли слой, который будем анимировать
    auto selectedItems = ui->tree_layers->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this,
                             "Внимание",
                             "Выберите слой (DataNode) в дереве, чтобы использовать его настройки для видео.");
        return;
    }
    QUuid baseNodeId = QUuid::fromString(selectedItems.first()->data(0, Qt::UserRole).toString());

    // 2. Выбираем файлы для анимации
    QStringList files = QFileDialog::getOpenFileNames(this, "Выберите файлы для анимации", "", "Bin Files (*.bin)");
    if (files.isEmpty())
        return;

    // 3. Выбираем куда сохранить видео
    QString savePath = QFileDialog::getSaveFileName(this, "Сохранить видео", "", "Video Files (*.ogv)");
    if (savePath.isEmpty())
        return;

    // TODO: Здесь можно добавить вызов QInputDialog для запроса `stride` (шага кадров) у пользователя
    int stride = 1;
    int fps    = 30;
    // 4. Показываем диалог загрузки и запускаем процесс
    m_exportProgressDialog->setValue(0);
    m_exportProgressDialog->show();

    m_app->startVideoExport(baseNodeId, files, savePath, stride, fps);
}
MainWindow::~MainWindow() {
    delete ui;
}
void MainWindow::on_action_resetCameraClicked() {
    if (m_renderer)
        m_renderer->resetCamera();
}
void MainWindow::on_action_ChangedViewClicked(QAction* action) {
    if (!m_renderer || !action)
        return;

    // Меняем камеру
    auto type = static_cast<Visualize::CameraViewType>(action->data().toInt());
    m_renderer->setCameraView(type);

    ui->action_view_top->setText(action->text());
    ui->action_view_top->setIcon(action->icon());
}
void MainWindow::on_action_ChangedBackgroundClicked(QAction* action) {
    if (!m_renderer || !action)
        return;

    // Смена цвета в рендерере
    if (action == ui->action_bg_black) {
        m_renderer->setBackgroundColor(0.0, 0.0, 0.0);
    } else if (action == ui->action_bg_white) {
        m_renderer->setBackgroundColor(0.3, 1.0, 1.0);
    }

    // Меняем текст на кнопке тулбара
    ui->action_bg_settings->setText(action->text());
    ui->action_bg_settings->setIcon(action->icon());
}
void MainWindow::on_action_toggleAxesChanged(bool checked) {
    if (m_renderer)
        m_renderer->setAxesVisible(checked);
}
void MainWindow::on_action_toggleGridChanged(bool checked) {
    if (m_renderer)
        m_renderer->setGridVisible(checked);
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
        IO::ReadScheme        scheme     = IO::SchemeFactory::createDefaultSheme(entityType, format);
        m_app->getDataManager()->importDataAsync(paths[0], scheme);
    } else {
        QList<QSpace::IO::BatchTask> tasks;
        for (const auto& path : paths) {
            IO::FileFormat        format     = IO::Utils::getFormat(path);
            Visualize::EntityType entityType = IO::Utils::getEntityType(path);
            IO::ReadScheme        scheme     = IO::SchemeFactory::createDefaultSheme(entityType, format);
            IO::BatchTask         task;
            task.path   = path;
            task.scheme = scheme;
            tasks.append(task);
        }
        m_app->getDataManager()->importBatchDataAsync(tasks);
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