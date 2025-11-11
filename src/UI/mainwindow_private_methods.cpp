#include <qcontainerfwd.h>
#include <qlistwidget.h>

#include <QMessageBox>
#include <QSettings>

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
    double xmin = ui->lineEdit_min_x->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси X"));
        return false;
    }
    double xmax = ui->lineEdit_max_x->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси X"));
        return false;
    }
    if (xmin >= xmax) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("минимальное значение по оси X не может быть больше или равно максимальному"));
        return false;
    }
    // считываем ограничения для Y
    double ymin = ui->lineEdit_min_y->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси Y"));
        return false;
    }
    double ymax = ui->lineEdit_max_y->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси Y"));
        return false;
    }
    if (ymin >= ymax) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("минимальное значение по оси Y не может быть больше или равно максимальному"));
        return false;
    }
    double zmin = ui->lineEdit_min_z->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси Z"));
        return false;
    }
    double zmax = ui->lineEdit_max_z->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси Z"));
        return false;
    }
    if (zmin >= zmax) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("минимальное значение по оси Z не может быть больше или равно максимальному"));
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
void MainWindow::setup_columns_comboBoxes(const int num_col)
{
    if (num_col <= 0 || num_col > 100) throw std::invalid_argument("число колонок должно быть от 1 до 100");

    QLayoutItem *item;
    if (ui->formLayout->rowCount() > 0) {
        while ((item = ui->formLayout_names_columns_ifile->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();  // Удаляем все виджеты (включая QLabel)
            }
            delete item;  // Удаляем элемент layout-а
        }
    }

    columnCombos.clear();
    columnCombos.reserve(num_col);
    for (int i = 0; i < num_col; ++i) {
        QLabel *label = new QLabel(tr(("Колонка " + std::to_string(i)).c_str()), ui->scrolContent_names_columns_ifile);
        QComboBox *combo = new QComboBox(ui->scrolContent_names_columns_ifile);
        for (const auto &i : ParametrsList::Columns_names) {
            combo->addItem(QString::fromStdString(i.data()));
        }
        combo->setCurrentIndex(i % ParametrsList::Columns_names.size());
        ui->formLayout_names_columns_ifile->addRow(label, combo);
        columnCombos.push_back(combo);
    }
    ui->formLayout_names_columns_ifile->update();
}
void MainWindow::setup_columns_comboBoxes_DM_S(const int num_col)
{
    if (num_col <= 0 || num_col > 100) throw std::invalid_argument("число колонок должно быть от 1 до 100");

    if (ui->formLayout_names_columns_ifile->rowCount() > 0) {
        while (ui->formLayout_names_columns_ifile->count() > 0) {
            QLayoutItem *item = ui->formLayout_names_columns_ifile->takeAt(0);
            if (item != nullptr) {
                if (QWidget *widget = item->widget()) {
                    widget->deleteLater();
                }
                delete item;
            }
        }
    }

    columnCombos.clear();
    columnCombos.reserve(num_col);

    for (int i = 0; i < num_col; ++i) {
        QLabel *label = new QLabel(tr(("Колонка " + std::to_string(i)).c_str()), ui->scrolContent_names_columns_ifile);
        QComboBox *combo = new QComboBox(ui->scrolContent_names_columns_ifile);
        for (const auto &i : ParametrsList::Columns_names_DM_S) {
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
            QLayoutItem *item = ui->formLayout_names_columns_ifile->takeAt(0);
            if (item != nullptr) {
                if (QWidget *widget = item->widget()) {
                    widget->deleteLater();
                }
                delete item;
            }
        }
    }

    columnCombos.clear();
    columnCombos.reserve(num_col);
    for (int i = 0; i < num_col; ++i) {
        QLabel *label = new QLabel(tr(("Колонка " + std::to_string(i)).c_str()), ui->scrolContent_names_columns_ifile);
        QComboBox *combo = new QComboBox(ui->scrolContent_names_columns_ifile);
        for (const auto &i : ParametrsList::Columns_names) {
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
    auto selected_items = ui->listWidget_input_file_names->selectedItems();
    if (selected_items.isEmpty()) {
        QMessageBox::information(this, "Удаление", "Выберите файл для удаления.");
        return;
    }

    for (int i = selected_items.size() - 1; i >= 0; --i) {
        QListWidgetItem *item = selected_items[i];
        QString file_path = item->text();
        inputfiles_names.removeOne(file_path);
        delete item;
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

    // === 5. Параметры сетки ===
    ui->lineEdit_min_x->setText(settings.value("grid/min_x", "-3").toString());
    ui->lineEdit_max_x->setText(settings.value("grid/max_x", "3").toString());
    ui->lineEdit_min_y->setText(settings.value("grid/min_y", "-3").toString());
    ui->lineEdit_max_y->setText(settings.value("grid/max_y", "3").toString());
    ui->lineEdit_min_z->setText(settings.value("grid/min_z", "-3").toString());
    ui->lineEdit_max_z->setText(settings.value("grid/max_z", "3").toString());
    ui->lineEdit_hb->setText(settings.value("grid/hb", "0.01").toString());

    // === 6. Константы (вкладка "Настройка Констант") ===
    ui->lineEdit_gamma->setText(settings.value("const/gamma", "1.666667").toString());
    ui->lineEdit_Km->setText(settings.value("const/Km", "3.72").toString());
    ui->lineEdit_Kr->setText(settings.value("const/Kr", "0.9").toString());
    ui->lineEdit_input->setText(settings.value("lineedits/input", "").toString());
    ui->lineEdit_output->setText(settings.value("lineedits/output", "").toString());
    // TODO: ... и так далее для всех lineEdit

    // === 7. QListWidget (выбранные файлы) ===
    ui->listWidget_input_file_names->clear();
    int fileCount = settings.beginReadArray("files/input");
    for (int i = 0; i < fileCount; ++i) {
        settings.setArrayIndex(i);
        ui->listWidget_input_file_names->addItem(settings.value("path").toString());
    }
    settings.endArray();

    // === 8. CheckBox ===
    ui->checkBox_column_customize->setChecked(settings.value("checkbox/custom_columns", false).toBool());

    ui->listWidget_input_file_names->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listWidget_input_file_names->setContextMenuPolicy(Qt::CustomContextMenu);

    // настройка listWidget
    ui->listWidget_input_file_names->installEventFilter(this);

    on_comboBox_output_data_params_changed(ui->comboBox_output_data_params->currentIndex());  // устанавливаем начальное
                                                                                              // состояние выходных
    on_lineEdit_Slice_changed("");                                                            // параметров
}
void MainWindow::connectSlots()
{
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
    /*--x min--*/ connect(ui->lineEdit_min_x, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--x max--*/ connect(ui->lineEdit_max_x, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--y min--*/ connect(ui->lineEdit_min_y, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--x max--*/ connect(ui->lineEdit_max_y, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--z min--*/ connect(ui->lineEdit_min_z, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--z max--*/ connect(ui->lineEdit_max_z, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);

    // редактирование выбранных файлов
    connect(ui->listWidget_input_file_names, &QListWidget::customContextMenuRequested, this,
            &MainWindow::on_listWidget_right_mouse_clicked);
}