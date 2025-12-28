#include <filesystem>
#include <qabstractitemmodel.h>
#include <qabstractitemview.h>
#include <qcontainerfwd.h>
#include <qlayoutitem.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qlistwidget.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qvectornd.h>
#include <qwidget.h>

#include <QMessageBox>
#include <QSettings>
#include <QtDataVisualization/Q3DScatter>
#include <QtDataVisualization/QScatter3DSeries>
#include <cstddef>
#include <memory>
#include <vector>

#include "./ui_mainwindow.h"
#include "ConstantsParametrs.h"
#include "mainwindow.h"
bool MainWindow::is_correct_data_QLineEdit()
{
    bool ok = true;
    double hb = ui->lineEdit_hb->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("размер ячейки должен быть вещественым числом"));
        return false;
    }
    if (!(hb > 0 && hb <= 1000)) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("размер ячейки должен принимать значения от 0 до 1000"));
        return false;
    }
    statusBar()->clearMessage();
    // считываем ограничения для X
    double xmin = ui->lineEdit_min_x_bound->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси X для области ограничения"));
        return false;
    }
    double xmax = ui->lineEdit_max_x_bound->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси X для области ограничения"));
        return false;
    }
    if (xmin >= xmax) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(
            tr("минимальное значение по оси X  для области ограничения не может быть больше или равно максимальному"));
        return false;
    }
    // считываем ограничения для Y
    double ymin = ui->lineEdit_min_y_bound->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси Y для области ограничения"));
        return false;
    }
    double ymax = ui->lineEdit_max_y_bound->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси Y для области ограничения"));
        return false;
    }
    if (ymin >= ymax) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(
            tr("минимальное значение по оси Y для области ограничения не может быть больше или равно максимальному"));
        return false;
    }
    double zmin = ui->lineEdit_min_z_bound->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси Z для области ограничения"));
        return false;
    }
    double zmax = ui->lineEdit_max_z_bound->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси Z для области ограничения"));
        return false;
    }
    if (zmin >= zmax) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(
            tr("минимальное значение по оси Z для области ограничения не может быть больше или равно максимальному"));
        return false;
    }
    // считываем ограничения для X
    double xmin_area = ui->lineEdit_min_x_lim->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси X для отображаемой области"));
        return false;
    }
    double xmax_area = ui->lineEdit_max_x_lim->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси X для отображаемой области"));
        return false;
    }
    if (xmin_area >= xmax_area) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(
            tr("минимальное значение по оси X для отображаемой области не может быть больше или равно максимальному"));
        return false;
    }
    // считываем ограничения для Y
    double ymin_area = ui->lineEdit_min_y_lim->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси Y для отображаемой области"));
        return false;
    }
    double ymax_area = ui->lineEdit_max_y_lim->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси Y для отображаемой области"));
        return false;
    }
    if (ymin_area >= ymax_area) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(
            tr("минимальное значение по оси Y для отображаемой области не может быть больше или равно максимальному"));
        return false;
    }
    double alpha = ui->lineEdit_angle_alpha->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести угол поворота вокруг оси X для области ограничения - alpha"));
        return false;
    }
    double beta = ui->lineEdit_angle_beta->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести угол поворота вокруг оси Y для области ограничения - beta"));
        return false;
    }
    double phi = ui->lineEdit_angle_phi->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести угол поворота вокруг оси Z для области ограничения - phi"));
        return false;
    }
    double teta = ui->lineEdit_angle_teta->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести угол наклона проекции - teta"));
        return false;
    }
    double psi = ui->lineEdit_angle_psi->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести угол поворота проекции - psi"));
        return false;
    }
    if (alpha < -180 || alpha > 180) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("некорректное значение угла alpha, должно быть от -180 до 180"));
        return false;
    }
    if (beta < -180 || beta > 180) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("некорректное значение угла beta, должно быть от -180 до 180"));
        return false;
    }
    if (phi < -180 || phi > 180) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("некорректное значение угла phi, должно быть от -180 до 180"));
        return false;
    }
    if (teta < -180 || teta > 180) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("некорректное значение угла teta, должно быть от -180 до 180"));
        return false;
    }
    if (psi < -180 || psi > 180) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("некорректное значение угла psi, должно быть от -180 до 180"));
        return false;
    }
    return true;
}
// устанавливает все константные параметры неизменяемыми
void MainWindow::set_False_Enable_Line_Edit_Constants()
{
    ui->lineEdit_gamma->setEnabled(false);
    ui->lineEdit_Courant->setEnabled(false);
    ui->lineEdit_alpha->setEnabled(false);
    ui->lineEdit_beta->setEnabled(false);
    ui->lineEdit_eta->setEnabled(false);
    ui->lineEdit_hpmax->setEnabled(false);
    ui->lineEdit_hpmin->setEnabled(false);
    ui->lineEdit_eps->setEnabled(false);
    ui->lineEdit_dtgrav->setEnabled(false);
    ui->lineEdit_Km->setEnabled(false);
    ui->lineEdit_Kr->setEnabled(false);
    ui->lineEdit_Zcool->setEnabled(false);
    ui->lineEdit_Tmin->setEnabled(false);
    ui->lineEdit_lfoton->setEnabled(false);
    ui->lineEdit_Heat_CR->setEnabled(false);
    ui->lineEdit_Collaps->setEnabled(false);
    ui->lineEdit_dt_MC->setEnabled(false);
    ui->lineEdit_dt_YS->setEnabled(false);
    ui->lineEdit_Tgas_YS->setEnabled(false);
}
void MainWindow::setup_columns_comboBoxes_DM_S(const int num_col)
{
    if (num_col <= 0 || num_col > 100) throw std::invalid_argument("число колонок должно быть от 1 до 100");

    if (ui->formLayout_names_columns_ifile->rowCount() > 0) {
        while (ui->formLayout_names_columns_ifile->count() > 0) {
            QLayoutItem* item = ui->formLayout_names_columns_ifile->takeAt(0);
            if (item != nullptr) {
                if (QWidget* widget = item->widget()) {
                    widget->deleteLater();
                }
                delete item;
            }
        }
    }

    columnCombos.clear();
    columnCombos.reserve(num_col);

    for (int i = 0; i < num_col; ++i) {
        QLabel* label = new QLabel(tr(("Колонка " + std::to_string(i)).c_str()), ui->scrolContent_names_columns_ifile);
        QComboBox* combo = new QComboBox(ui->scrolContent_names_columns_ifile);
        for (const auto& i : ParametrsList::Columns_names_DM_S) {
            combo->addItem(QString::fromStdString(i.data()));
        }
        combo->setCurrentIndex(i % ParametrsList::Columns_names_DM_S.size());
        ui->formLayout_names_columns_ifile->addRow(label, combo);
        columnCombos.push_back(combo);
    }
    ui->formLayout_names_columns_ifile->update();
}

void MainWindow::setup_columns_comboBoxes_G_MC_YS(const int num_col)
{
    if (num_col <= 0 || num_col > 100) throw std::invalid_argument("число колонок должно быть от 1 до 100");

    if (ui->formLayout_names_columns_ifile->rowCount() > 0) {
        while (ui->formLayout_names_columns_ifile->count() > 0) {
            QLayoutItem* item = ui->formLayout_names_columns_ifile->takeAt(0);
            if (item != nullptr) {
                if (QWidget* widget = item->widget()) {
                    widget->deleteLater();
                }
                delete item;
            }
        }
    }

    columnCombos.clear();
    columnCombos.reserve(num_col);
    for (int i = 0; i < num_col; ++i) {
        QLabel* label = new QLabel(tr(("Колонка " + std::to_string(i)).c_str()), ui->scrolContent_names_columns_ifile);
        QComboBox* combo = new QComboBox(ui->scrolContent_names_columns_ifile);
        for (const auto& i : ParametrsList::Columns_names) {
            combo->addItem(QString::fromStdString(i.data()));
        }
        combo->setCurrentIndex(i % ParametrsList::Columns_names.size());
        ui->formLayout_names_columns_ifile->addRow(label, combo);
        columnCombos.push_back(combo);
    }
    ui->formLayout_names_columns_ifile->update();
}
void MainWindow::delete_selected_files()
{
    QListWidget* activeList = nullptr;
    if (ui->tabWidget->currentWidget() == ui->tab_3) {
        activeList = ui->listWidget_input_file_names;
    } else if (ui->tabWidget->currentWidget() == ui->tab_6) {
        activeList = ui->listWidget_input_files;
    }
    if (!activeList || activeList->selectedItems().isEmpty()) {
        QMessageBox::information(this, "Удаление", "Выберите файл для удаления.");
        return;
    }
    QStringList toRemove;
    for (QListWidgetItem* item : activeList->selectedItems()) {
        toRemove << item->text();
    }

    for (const QString& path : toRemove) {
        inputfiles_names.removeOne(path);
    }

    QList<QListWidget*> allLists = {ui->listWidget_input_file_names, ui->listWidget_input_files};
    for (QListWidget* list : allLists) {
        for (int i = list->count() - 1; i >= 0; --i) {
            if (toRemove.contains(list->item(i)->text())) {
                delete list->takeItem(i);
            }
        }
    }
}
void MainWindow::loadSettings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);

    // === 1. Окно ===
    if (settings.contains("window/geometry")) {
        restoreGeometry(settings.value("window/geometry").toByteArray());
        restoreState(settings.value("window/state").toByteArray());
    } else {
        // Первый запуск — нормальный размер
        resize(1150, 750);
    }

    // === 2. Вкладки ===
    int tabIndex = settings.value("tabs/current", 0).toInt();
    if (tabIndex >= 0 && tabIndex < ui->tabWidget->count()) ui->tabWidget->setCurrentIndex(tabIndex);

    // === 3. Восстановление путей ===
    inputfiles_names = settings.value("files/input", QStringList()).toStringList();
    outputDir = settings.value("files/output_dir", "").toString();
    // === 4. ComboBox'ы ===
    ui->comboBox->setCurrentIndex(settings.value("combos/output_type", 0).toInt());
    ui->comboBox_output_data_params->setCurrentIndex(settings.value("combos/data_type", 0).toInt());
    ui->comboBox_X->setCurrentIndex(settings.value("combos/X", 0).toInt());
    ui->comboBox_Y->setCurrentIndex(settings.value("combos/Y", 0).toInt());
    ui->comboBox_Z->setCurrentIndex(settings.value("combos/Z", 0).toInt());
    ui->comboBox_type_structures_ifiles->setCurrentIndex(settings.value("combos/file_structure", 0).toInt());

    // === 5. Параметры области ограничения ===
    ui->lineEdit_min_x_bound->setText(settings.value("boundary/min_x", "-3").toString());
    ui->lineEdit_max_x_bound->setText(settings.value("boundary/max_x", "3").toString());
    ui->lineEdit_min_y_bound->setText(settings.value("boundary/min_y", "-3").toString());
    ui->lineEdit_max_y_bound->setText(settings.value("boundary/max_y", "3").toString());
    ui->lineEdit_min_z_bound->setText(settings.value("boundary/min_z", "-3").toString());
    ui->lineEdit_max_z_bound->setText(settings.value("boundary/max_z", "3").toString());
    ui->lineEdit_angle_alpha->setText(settings.value("boundary/alpha", "0").toString());
    ui->lineEdit_angle_beta->setText(settings.value("boundary/beta", "0").toString());
    ui->lineEdit_angle_phi->setText(settings.value("boundary/phi", "0").toString());

    // === 6. Параметры сетки ===
    ui->lineEdit_min_x_lim->setText(settings.value("grid/min_x", "-3").toString());
    ui->lineEdit_max_x_lim->setText(settings.value("grid/max_x", "3").toString());
    ui->lineEdit_min_y_lim->setText(settings.value("grid/min_y", "-3").toString());
    ui->lineEdit_max_y_lim->setText(settings.value("grid/max_y", "3").toString());
    ui->lineEdit_angle_teta->setText(settings.value("grid/teta", "0").toString());
    ui->lineEdit_angle_psi->setText(settings.value("grid/psi", "0").toString());

    ui->lineEdit_hb->setText(settings.value("grid/hb", "0.01").toString());
    // === 7. Константы (вкладка "Настройка Констант") ===
    ui->lineEdit_gamma->setText(settings.value("const/gamma", "1.666667").toString());
    ui->lineEdit_Km->setText(settings.value("const/Km", "3.72").toString());
    ui->lineEdit_Kr->setText(settings.value("const/Kr", "0.9").toString());
    ui->lineEdit_input->setText(settings.value("lineedits/input", "").toString());
    ui->lineEdit_output->setText(settings.value("lineedits/output", "").toString());
    // TODO: ... и так далее для всех lineEdit

    // === 8. QListWidget (выбранные файлы) ===
    ui->listWidget_input_file_names->clear();
    ui->listWidget_input_files->clear();
    int fileCount = settings.beginReadArray("files/input");
    for (int i = 0; i < fileCount; ++i) {
        settings.setArrayIndex(i);
        ui->listWidget_input_file_names->addItem(settings.value("path").toString());
        ui->listWidget_input_files->addItem(settings.value("path").toString());
    }
    settings.endArray();

    // === 9. CheckBox ===
    ui->checkBox_column_customize->setChecked(settings.value("checkbox/custom_columns", false).toBool());

    ui->listWidget_input_file_names->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listWidget_input_file_names->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listWidget_input_files->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listWidget_input_files->setContextMenuPolicy(Qt::CustomContextMenu);

    // настройка listWidget
    ui->listWidget_input_file_names->installEventFilter(this);
    ui->listWidget_input_files->installEventFilter(this);
    on_comboBox_output_data_params_changed(ui->comboBox_output_data_params->currentIndex());  // устанавливаем начальное
                                                                                              // состояние выходных
    on_lineEdit_Slice_changed("");                                                            // параметров
    on_lineEdit_projection_area_changed("");
}
void MainWindow::connectSlots()
{
    connect(ui->pushButton_add_files, &QPushButton::clicked, this, &MainWindow::on_pushButton_add_input_file_clicked);
    connect(ui->pushButton_variable_files, &QPushButton::clicked, this, &MainWindow::on_pushButton_input_files_clicked);

    connect(ui->pushButtonconvert_1, &QPushButton::clicked, this, &MainWindow::on_pushButtonconvert_clicked);
    connect(ui->pushButtonconvert_2, &QPushButton::clicked, this, &MainWindow::on_pushButtonconvert_clicked);
    connect(ui->pushButtonconvert_3, &QPushButton::clicked, this, &MainWindow::on_pushButtonconvert_clicked);
    connect(ui->action_reset_settings, &QAction::triggered, this, &MainWindow::on_action_reset_settings);
    // настройка сигнала кнопки входных файлов
    connect(ui->pushButton_input_files1, &QPushButton::clicked, this, &MainWindow::on_pushButton_input_files_clicked);
    // on_comboBox_output_data_params_changed - меняет список разрешенных параметров вывода
    connect(ui->comboBox_output_data_params, &QComboBox::currentIndexChanged, this,
            &MainWindow::on_comboBox_output_data_params_changed);  // соединение элемента comboBox_output_data_params с
                                                                   // слотом on_comboBox_output_data_params_changed

    // устанавливает число колонок по умолчанию в файле
    connect(ui->lineEdit_numbers_columns_ifiles, &QLineEdit::textChanged, this,  // соединяем элемент
                                                                                 // lineEdit_numbers_columns_ifiles с слотом
                                                                                 // on_lineEdit_numbers_columns_files_changed
            &MainWindow::on_lineEdit_numbers_columns_files_changed);

    // разрешение на редактирование числа колонок в файле
    connect(ui->checkBox_column_customize, &QCheckBox::checkStateChanged, this,
            &MainWindow::on_checkbox_column_customise_change);
    connect(ui->lineEdit_hb, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);

    /*--x min_bound--*/ connect(ui->lineEdit_min_x_bound, &QLineEdit::textChanged, this,
                                &MainWindow::on_lineEdit_Slice_changed);
    /*--x max_bound--*/ connect(ui->lineEdit_max_x_bound, &QLineEdit::textChanged, this,
                                &MainWindow::on_lineEdit_Slice_changed);
    /*--y min_bound--*/ connect(ui->lineEdit_min_y_bound, &QLineEdit::textChanged, this,
                                &MainWindow::on_lineEdit_Slice_changed);
    /*--y max_bound--*/ connect(ui->lineEdit_max_y_bound, &QLineEdit::textChanged, this,
                                &MainWindow::on_lineEdit_Slice_changed);
    /*--z min_bound--*/ connect(ui->lineEdit_min_z_bound, &QLineEdit::textChanged, this,
                                &MainWindow::on_lineEdit_Slice_changed);
    /*--z max_bound--*/ connect(ui->lineEdit_max_z_bound, &QLineEdit::textChanged, this,
                                &MainWindow::on_lineEdit_Slice_changed);

    /*--x min_lim--*/ connect(ui->lineEdit_min_x_lim, &QLineEdit::textChanged, this,
                              &MainWindow::on_lineEdit_projection_area_changed);
    /*--x max_lim--*/ connect(ui->lineEdit_max_x_lim, &QLineEdit::textChanged, this,
                              &MainWindow::on_lineEdit_projection_area_changed);
    /*--y min_lim--*/ connect(ui->lineEdit_min_y_lim, &QLineEdit::textChanged, this,
                              &MainWindow::on_lineEdit_projection_area_changed);
    /*--y max_lim--*/ connect(ui->lineEdit_max_y_lim, &QLineEdit::textChanged, this,
                              &MainWindow::on_lineEdit_projection_area_changed);
    /*--hb --*/ connect(ui->lineEdit_hb, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_projection_area_changed);

    /*-- alpha --*/ connect(ui->lineEdit_angle_alpha, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*-- beta --*/ connect(ui->lineEdit_angle_beta, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*-- phi --*/ connect(ui->lineEdit_angle_phi, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);

    /*-- teta --*/ connect(ui->lineEdit_angle_teta, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*-- teta --*/ connect(ui->lineEdit_angle_psi, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    // редактирование выбранных файлов
    connect(ui->listWidget_input_file_names, &QListWidget::customContextMenuRequested, this,
            &MainWindow::on_listWidget_right_mouse_clicked);
    connect(ui->listWidget_input_files, &QListWidget::customContextMenuRequested, this,
            &MainWindow::on_listWidget_right_mouse_clicked);
    // connect(ui->listWidget_input_files, &QListWidget::itemClicked, this,
    //         &MainWindow::on_list_widget_input_files_item_clicked);
}
std::vector<std::string> MainWindow::get_columns_for_file_from_UI(){
    // преобразование combobox в vector<string> для колонок
    std::vector<std::string> columns;
        columns.reserve(columnCombos.size());
        for (const auto& col : columnCombos) {
            columns.push_back(col->currentText().toStdString());
        }
    return columns;
}
std::vector<std::filesystem::path> MainWindow::get_ifiles_pathes_from_UI(){
    // преобразование QString в filesystem::path
        std::vector<std::filesystem::path> ifiles_names;
        ifiles_names.reserve(inputfiles_names.size());
        for (const auto& ifile_name : inputfiles_names) {
            ifiles_names.push_back(ifile_name.toStdString());
        }
        return ifiles_names;
}