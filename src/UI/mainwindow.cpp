#include "mainwindow.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qdir.h>
#include <qkeysequence.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qnamespace.h>

#include <QFileDialog>
#include <QFormLayout>
#include <QMessageBox>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "./ui_mainwindow.h"
#include "Converter/Converter.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);  // установка начальных настроек интерфейса

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
    connect(ui->lineEdit_hb, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_hb_changed);
    on_comboBox_output_data_params_changed(ui->comboBox_output_data_params->currentIndex());  // устанавливаем начальное
                                                                                              // состояние выходных
    on_lineEdit_hb_changed(ui->lineEdit_hb->text());                                          // параметров
}
void MainWindow::on_lineEdit_hb_changed(const QString &changed_line)
{
    if (!changed_line.isEmpty()) {
        bool ok = true;
        double hb = changed_line.toDouble(&ok);
        if (!ok) {
            statusBar()->setStyleSheet("QStatusBar { color: red; }");
            statusBar()->showMessage(tr("размер ячейки должен быть вещественым числом"));
            return;
        }
        if (!(hb > 0 && hb <= 1000)) {
            statusBar()->setStyleSheet("QStatusBar { color: red; }");
            statusBar()->showMessage(tr("размер ячейки должен принимать значения от 0 до 1000"));
            return;
        }
        statusBar()->clearMessage();
        // создаем список выбора типов колонок
        // считываем ограничения для X
        double xmin = ui->lineEdit_min_x->text().toDouble(&ok);
        if (!ok) {
            statusBar()->setStyleSheet("QStatusBar { color: red; }");
            statusBar()->showMessage(tr("нужно ввести минимальное значение по оси X"));
            return;
        }
        double xmax = ui->lineEdit_max_x->text().toDouble(&ok);
        if (!ok) {
            statusBar()->setStyleSheet("QStatusBar { color: red; }");
            statusBar()->showMessage(tr("нужно ввести максимальное значение по оси X"));
            return;
        }
        // считываем ограничения для Y
        double ymin = ui->lineEdit_min_y->text().toDouble(&ok);
        if (!ok) {
            statusBar()->setStyleSheet("QStatusBar { color: red; }");
            statusBar()->showMessage(tr("нужно ввести минимальное значение по оси Y"));
            return;
        }
        double ymax = ui->lineEdit_max_y->text().toDouble(&ok);
        if (!ok) {
            statusBar()->setStyleSheet("QStatusBar { color: red; }");
            statusBar()->showMessage(tr("нужно ввести максимальное значение по оси Y"));
            return;
        }
        // вычисляем точки
        int Nbx = static_cast<int>((xmax - xmin + hb) / hb + 0.5);
        int Nby = static_cast<int>((ymax - ymin + hb) / hb + 0.5);
        ui->lineEdit_Nx->setText(std::to_string(Nbx).c_str());
        ui->lineEdit_Ny->setText(std::to_string(Nby).c_str());
    }
}
MainWindow::~MainWindow() {}
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
// задает разрашенные выходные параметры для записи, устанавливает разрешенные константы изменяемыми, задает число колонок по
// умолчанию в файле
void MainWindow::on_comboBox_output_data_params_changed(int index)  // управляет списком выходных записываемых параметров
{
    QString curent_param = ui->comboBox_output_data_params->itemText(index);  // получаем значение типа выходных данных
    ui->comboBox_Z->clear();                                                  // очищаем список выбора типа выходных данных
    for (const auto &item : ParametrsList::Z_outParams) {  // цикл по списку разрешенных параметров записи в файл
        ui->comboBox_Z->addItem(QString(item.data()));
    }
    ui->comboBox_Z->addItem(QString(ParametrsList::Z_outParamKit.data()));  // пользовательский набор записываемых выходных
                                                                            // параметров
    const auto &PL_NoTermal = ParametrsList::NoTermal_odata_t;
    if (std::find(PL_NoTermal.begin(), PL_NoTermal.end(), curent_param) == PL_NoTermal.end()) {  // добавление параметров
                                                                                                 // связанных с газом если
                                                                                                 // соответствет типы
                                                                                                 // выхожных данных
        ui->comboBox_Z->addItem(QString(ParametrsList::Z_outParamT.data()));
        ui->comboBox_Z->addItem(QString(ParametrsList::Z_outParamLgT.data()));
    }
    set_False_Enable_Line_Edit_Constants();
    // разрешение на редактирование констант и установка числа колонок в файлах
    if (curent_param == ParametrsList::DarkMatter_type || curent_param == ParametrsList::Stars_type) {
        ui->lineEdit_Km->setEnabled(true);
        ui->lineEdit_Kr->setEnabled(true);
        if (!ui->checkBox_column_customize->checkState()) {
            ui->lineEdit_numbers_columns_ifiles->setText("7");
            setup_columns_comboBoxes(7);
        }
    } else if (curent_param == ParametrsList::Gas_type || curent_param == ParametrsList::YongStars_type ||
               curent_param == ParametrsList::MolecularClouds_type) {
        ui->lineEdit_gamma->setEnabled(true);
        ui->lineEdit_Km->setEnabled(true);
        ui->lineEdit_Kr->setEnabled(true);
        if (!ui->checkBox_column_customize->checkState()) {
            ui->lineEdit_numbers_columns_ifiles->setText("11");
            setup_columns_comboBoxes(11);
        }
    }
}
// проверяем что записано разрешенное число колонок в lineEdit_numbers_columns_files
void MainWindow::on_lineEdit_numbers_columns_files_changed(const QString &changed_line)
{
    if (!changed_line.isEmpty()) {
        bool ok = true;
        int column_size = changed_line.toInt(&ok);
        if (!ok) {
            statusBar()->setStyleSheet("QStatusBar { color: red; }");
            statusBar()->showMessage(tr("число колонок должно быть целым  от 1 до 100"));
            return;
        }
        if (!(column_size > 0 && column_size <= 100)) {
            statusBar()->setStyleSheet("QStatusBar { color: red; }");
            statusBar()->showMessage(tr("число колонок должно быть целым  от 1 до 100"));
            return;
        }
        statusBar()->clearMessage();
        // создаем список выбора типов колонок
        setup_columns_comboBoxes(column_size);
    }
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
// разрешает или запрещает редактировать число колонок в файле
void MainWindow::on_checkbox_column_customise_change(Qt::CheckState state)
{
    if (state == Qt::CheckState::Checked) {
        ui->lineEdit_numbers_columns_ifiles->setEnabled(true);
    } else if (state == Qt::CheckState::Unchecked) {
        ui->lineEdit_numbers_columns_ifiles->setEnabled(false);
    }
}
void MainWindow::on_pushButton_txt_clicked()
{
    try {
        QStringList inputfileNames =
            QFileDialog::getOpenFileNames(this, tr("Select TXT Files"), "", tr("Text Files (*.txt)"));

        //         // если выходной путь не выбран автоматически выбирается корневая директория входных файлов/output
        //         if (ui->lineEdit_output->text().toStdString().empty()) {
        //             QDir current_dir = QDir::currentPath();
        //             QString outputfileName = "output";
        //             QString new_output_folder_name = current_dir.absolutePath() + "/" + outputfileName;
        //             outputDir = new_output_folder_name;
        //             ui->lineEdit_output->setText(outputDir);
        //         }

        // // Проверяем ui->scrollContent
        // if (!ui->widget_2) {
        //     qDebug() << "Error: ui->scrollContent is null!";
        //     QMessageBox::critical(this, tr("Ошибка"), tr("ui->scrollContent не инициализирован!"));
        //     return;
        // }

        // Проверяем, есть ли layout_columns
        // QLayout *layout = ui->formLayout_columns;
        // if (!layout) {
        //     qDebug() << "Error: ui->scrollContent has no layout!";
        //     QMessageBox::critical(this, tr("Ошибка"), tr("ui->scrollContent не имеет layout!"));
        //     return;
        // }

        // QFormLayout *columnsLayout = qobject_cast<QFormLayout *>(layout);
        // if (!columnsLayout) {
        //     qDebug() << "Error: Layout is not QFormLayout, actual type:" << layout->metaObject()->className();
        //     std::string ss = layout->metaObject()->className();
        //     std::string ss1 = "Layout не является QFormLayout! актуальный тип:";
        //     ss1 += ss;
        //     QMessageBox::critical(this, tr("Ошибка"), tr(ss1.c_str()));
        //     return;
        // }

        // QLayoutItem *item;
        // while ((item = columnsLayout->takeAt(0)) != nullptr) {
        //     if (QWidget *widget = item->widget()) {
        //         widget->deleteLater();  // Безопасное удаление виджета
        //     }
        //     delete item;  // Удаляем QLayoutItem
        // }

        //             QString first_file = inputfileNames.first();
        //             if (!first_file.isEmpty()) {
        //                 Converter<double> first_converter;
        // #ifdef _WIN32
        //                 first_translator.load_input_file(std::filesystem::path(first_file.toStdU16String()));
        // #else
        //                 first_converter.load_input_file(std::filesystem::path(first_file.toStdString()));
        // #endif
        //                 // Получите число колонок
        //                 int numColumns = first_converter.get_column_count();

        //                 // елси файл не содержит колонок
        //                 if (numColumns <= 0) {
        //                     std::string file_path = first_file.toStdString();
        //                     std::string text = "Файл пуст или не содержит колонок: ";
        //                     QMessageBox::warning(this, tr("Ошибка"), tr(std::string(text + file_path).c_str()));
        //                     return;
        //                 }
        //                 if (numColumns != columnCombos.size()) {
        //                     columnCombos.clear();
        //                     auto optionsStd = Converter<double>::get_columns();
        //                     QStringList options;
        //                     for (const auto &opt : optionsStd) {
        //                         options.append(QString::fromStdString(std::string(opt)));
        //                     }

        // for (int i = 0; i < numColumns; ++i) {
        //     QLabel *label = new QLabel(tr("Колонка %1:").arg(i + 1), this);
        //     QComboBox *combo = new QComboBox(this);
        //     combo->addItems(options);
        //     combo->setCurrentIndex(options.indexOf("None"));  // По умолчанию "None"
        //     columnsLayout->addRow(label, combo);
        //     columnCombos.append(combo);
        // }
        //                 }

        //                 first_converter.clear();
        //                 for (auto &inputfileName : inputfileNames) {
        //                     // если  входной путь не пустой
        //                     if (!inputfileName.isEmpty()) {
        //                         Converter<double> item_converter;
        // #ifdef _WIN32
        //                         item_translator.load_input_file(std::filesystem::path(inputfileName.toStdU16String()));
        // #else
        //                         item_converter.load_input_file(std::filesystem::path(inputfileName.toStdString()));
        // #endif
        //                         // Получите число колонок
        //                         int numColumns_i = item_converter.get_column_count();

        //                         // елси файл не содержит колонок
        //                         if (numColumns_i <= 0) {
        //                             std::string file_path = inputfileName.toStdString();
        //                             std::string text = "Файл пуст или не содержит колонок: ";
        //                             QMessageBox::warning(this, tr("Ошибка"), tr(std::string(text + file_path).c_str()));
        //                             return;
        //                         }
        //                         if (numColumns_i != numColumns) {
        //                             std::string file_path = inputfileName.toStdString();
        //                             std::string first_file_path = first_file.toStdString();
        //                             std::string text = "число колонок в файлах не совпадают: ";
        //                             QMessageBox::warning(this, tr("Ошибка"),
        //                                                  tr(std::string(text + file_path + " != " +
        //                                                  first_file_path).c_str()));
        //                             return;
        //                         }
        //                     }
        //                 }
        //             }
        //         }
    } catch (const char *ex) {
        QMessageBox::warning(this, tr("Ошибка"), tr(ex));
        return;
    }
}
void MainWindow::on_pushButtongrd_clicked()
{
    QString outputfolderName = QFileDialog::getSaveFileName(this, tr("Select GRD Folder"));
    if (!outputfolderName.isEmpty()) {
        ui->lineEdit_output->setText(outputfolderName);
        outputDir = outputfolderName;
    }
    QDir new_dir;
    if (new_dir.mkpath(outputDir)) {
        outputDir = outputfolderName;
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось создать папку. Проверь права доступа.");
    }
}

void MainWindow::on_pushButtonconvert_clicked()
{
    //     try {
    //         QDir dir;
    //         if (!dir.exists(outputDir)) {
    //             if (!dir.mkpath(outputDir)) {
    //                 QMessageBox::warning(this, "Ошибка", "Не удалось создать папку. Проверь права доступа.");
    //             }
    //         }
    //         for (auto &inputfileName : inputFiles) {
    //             QFileInfo fileInfo(inputfileName);                                                // Извлекаем информацию
    //             о файле QString outputfileName = outputDir + "/" + fileInfo.completeBaseName() + ".grd";  // Имя без
    //             расширения + .grd converters.push_back(Converter<double>()); auto &item_converter = converters.back();
    // #ifdef _WIN32
    //             item_converter.load_input_file(std::filesystem::path(inputfileName.toStdU16String()));
    //             item_converter.load_output_file(outputfileName.toStdU16String());
    // #else
    //             item_converter.load_input_file(std::filesystem::path(inputfileName.toStdString()));
    //             item_converter.load_output_file(outputfileName.toStdString());
    // #endif
    //         }
    //     } catch (const std::exception &e) {
    //         QMessageBox::critical(this, tr("Ошибка"), QString("Ошибка открытия выходного файла: %1").arg(e.what()));
    //     }

    //     std::vector<std::string> columnMappings;  // вектор колонок
    //     std::vector<std::string> xyzNames;        // вектор координат в grd файле
    //     for (auto &combo : columnCombos) {
    //         QString value = combo->currentText();
    //         columnMappings.push_back(value.toStdString());
    //     }
    //     QString xyz_value = ui->comboBox_X->currentText().replace(" (координата)", "");
    //     xyzNames.push_back(xyz_value.toStdString());
    //     xyz_value = ui->comboBox_Y->currentText().replace(" (координата)", "");
    //     xyzNames.push_back(xyz_value.toStdString());
    //     xyz_value = ui->comboBox_Z->currentText();
    //     if (xyz_value.toStdString() == "density (плотность)")
    //         xyzNames.push_back("density");
    //     else if (xyz_value.toStdString() == "Termal (температура)")
    //         xyzNames.push_back("Termal");

    //     if (columnMappings.empty()) {
    //         QMessageBox::warning(this, tr("Ошибка"), tr("Не выбраны колонки для маппинга."));
    //         return;
    //     }
    //     if (xyzNames.size() != 3) {
    //         QMessageBox::warning(this, tr("Ошибка"), tr("Должны быть выбраны x, y, z."));
    //         return;
    //     }

    //     // Получите лимиты
    //     bool ok;
    //     double xMin = ui->lineEdit_min_x->text().toDouble(&ok);
    //     if (!ok) {
    //         QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный X Min"));
    //         return;
    //     }
    //     double xMax = ui->lineEdit_max_x->text().toDouble(&ok);
    //     if (!ok) {
    //         QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный X Max"));
    //         return;
    //     }
    //     double yMin = ui->lineEdit_min_y->text().toDouble(&ok);
    //     if (!ok) {
    //         QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Y Min"));
    //         return;
    //     }
    //     double yMax = ui->lineEdit_max_y->text().toDouble(&ok);
    //     if (!ok) {
    //         QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Y Max"));
    //         return;
    //     }
    //     // double zMin = ui->zMinLineEdit->text().toDouble(&ok);
    //     // if (!ok) { QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Z Min")); return; }
    //     // double zMax = ui->zMaxLineEdit->text().toDouble(&ok);
    //     // if (!ok) { QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Z Max")); return; }
    //     int nx = ui->lineEdit_Nx->text().toInt(&ok);
    //     if (!ok || nx <= 0) {
    //         QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Nx"));
    //         return;
    //     }
    //     int ny = ui->lineEdit_Ny->text().toInt(&ok);
    //     if (!ok || ny <= 0) {
    //         QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Ny"));
    //         return;
    //     }

    //     try {
    //         for (size_t i = 0; i < converters.size(); ++i) {
    //             // Настройка Converter
    //             converters[i].setup_columns(columnMappings, xyzNames);
    //             converters[i].setup_gridXYZ({xMin, xMax}, {yMin, yMax}, {0, 1}, {nx, ny});
    //             converters[i].read_input_file();
    //             converters[i].translate_to_grd_bindings();
    //             converters[i].clear_output_file_state();
    //         }
    //         QMessageBox::information(this, tr("Успех"), tr("Конвертация завершена."));
    //     } catch (const std::exception &e) {
    //         QMessageBox::critical(this, tr("Ошибка"), QString("Конвертация не удалась: %1").arg(e.what()));
    //     }
}
