#include "mainwindow.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qkeysequence.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpaintdevice.h>

#include <QFileDialog>
#include <QFormLayout>
#include <QMessageBox>
#include <algorithm>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "./ui_mainwindow.h"
#include "Converter/Converter_DarkMatter.h"
#include "Converter/Converter_Gas.h"
#include "Converter/Converter_MolecularClouds.h"
#include "Converter/Converter_Stars.h"
#include "Converter/Converter_YoungStars.h"

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

        if (inputfileNames.isEmpty()) {
            return;
        }
        // если выходной путь не выбран автоматически выбирается корневая директория входных файлов/output
        if (ui->lineEdit_output->text().toStdString().empty()) {
            QDir current_dir = QDir::currentPath();
            QString outputfileName = "output";
            QString new_output_folder_name = current_dir.absolutePath() + "/" + outputfileName;
            outputDir = new_output_folder_name;
            ui->lineEdit_output->setText(outputDir);
        }

        // проверяем что все файлы существуют
        for (const auto &file_name : inputfileNames) {
            QFileInfo file;
            if (!file.exists(file_name)) {
                QMessageBox::warning(this, tr("Ошибка"), tr("Файл не существует"));
                return;
            }
        }
        QFileInfo file(inputfileNames.back());
        QString parentDir = QDir::toNativeSeparators(file.absolutePath());

        ui->lineEdit_input->setText(parentDir);
        inputfiles_names = inputfileNames;
    } catch (const char *ex) {
        QMessageBox::warning(this, tr("Ошибка"), tr(ex));
        return;
    }
}
void MainWindow::on_pushButtongrd_clicked()
{
    QString outputfolderName = QFileDialog::getSaveFileName(this, tr("Select GRD Folder"));
    if (outputfolderName.isEmpty()) {
        return;
    }
    ui->lineEdit_output->setText(outputfolderName);
    outputDir = outputfolderName;
    QDir new_dir;
    if (!new_dir.mkpath(outputDir)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось создать папку. Проверь права доступа."));
    }
}

void MainWindow::on_pushButtonconvert_clicked()
{
    if (inputfiles_names.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не выбраны файлы для преобразования"));
        return;
    }
    if (ui->lineEdit_output->text().isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("выходной файл не задан"));
        return;
    } else {
        QDir new_dir;
        if (!new_dir.mkpath(outputDir)) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось создать папку. Проверь права доступа."));
        }
    }
    try {
        // преобразование QString в filesystem::path
        std::vector<std::filesystem::path> ifiles_names;
        ifiles_names.reserve(inputfiles_names.size());
        for (const auto &ifile_name : inputfiles_names) {
            ifiles_names.push_back(ifile_name.toStdString());
        }

        //----------------------------------------------------------------------------
        DataStorage datastorage(ifiles_names);  // создание хранилища данных

        // преобразование combobox в vector<int> для колонок
        std::vector<int> columns;
        columns.reserve(columnCombos.size());
        for (const auto &col : columnCombos) {
            columns.push_back(col->currentIndex());
        }

        datastorage.setup_columns(columns);  // задаем колонки в входных файлах
        datastorage.set_buff_size(4);        // устанавливаем число файлов записываемых за раз на диск
        ParametrsList::iniConstants c;
        count_cell Nbxy;
        //---------------задание констант-----------------
        c.gamma = ui->lineEdit_gamma->text().toDouble();
        c.hb = ui->lineEdit_hb->text().toDouble();
        c.Km = ui->lineEdit_Km->text().toDouble();
        c.Kr = ui->lineEdit_Kr->text().toDouble();

        //---------------задание размеров сетки--------------
        Nbxy.Nx = ui->lineEdit_Nx->text().toInt();
        Nbxy.Ny = ui->lineEdit_Ny->text().toInt();
        lim<double> x;
        lim<double> y;
        lim<double> z;
        x.max = ui->lineEdit_max_x->text().toDouble();
        x.min = ui->lineEdit_min_x->text().toDouble();
        y.max = ui->lineEdit_max_y->text().toDouble();
        y.min = ui->lineEdit_min_y->text().toDouble();
        z.max = ui->lineEdit_max_z->text().toDouble();
        z.min = ui->lineEdit_min_z->text().toDouble();

        //------------------------------создание конвертера------------------------------------------
        if (ui->comboBox_output_data_params->currentText() == ParametrsList::DarkMatter_type) {  // Темная Материя
            Converter_DarkMatter cdm(datastorage, c, Nbxy);
            cdm.set_limits(x, y, z);
            cdm.setup_output_data(ui->comboBox_Z->currentText().toStdString());
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_grd) {  // если файлы бинарные
                datastorage.load_file_metadate_bin();
                while (datastorage.readw_bin()) {
                    cdm.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cdm.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cdm.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_grd) {
                datastorage.load_file_metadate_txt();
                while (datastorage.readw_txt()) {
                    cdm.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cdm.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cdm.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                return;
            }
        } else if (ui->comboBox_output_data_params->currentText() == ParametrsList::Gas_type) {  // газовые облака
            Converter_Gas cg(datastorage, c, Nbxy);
            cg.set_limits(x, y, z);
            cg.setup_output_data(ui->comboBox_Z->currentText().toStdString());
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_grd) {
                datastorage.load_file_metadate_bin();
                while (datastorage.readw_bin()) {
                    cg.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cg.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cg.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_grd) {
                datastorage.load_file_metadate_txt();
                while (datastorage.readw_txt()) {
                    cg.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cg.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cg.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                return;
            }
        } else if (ui->comboBox_output_data_params->currentText() == ParametrsList::MolecularClouds_type) {  // Молекулярные
                                                                                                             // облака
            // создание конвертера
            Converter_MolecularClouds cmc(datastorage, c, Nbxy);
            cmc.set_limits(x, y, z);
            cmc.setup_output_data(ui->comboBox_Z->currentText().toStdString());
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_grd) {
                datastorage.load_file_metadate_bin();
                while (datastorage.readw_bin()) {
                    cmc.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cmc.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cmc.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_grd) {
                datastorage.load_file_metadate_txt();
                while (datastorage.readw_txt()) {
                    cmc.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cmc.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cmc.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                return;
            }
        } else if (ui->comboBox_output_data_params->currentText() == ParametrsList::Stars_type) {  // Звезды
            Converter_Stars cs(datastorage, c, Nbxy);
            cs.set_limits(x, y, z);
            cs.setup_output_data(ui->comboBox_Z->currentText().toStdString());
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_grd) {
                datastorage.load_file_metadate_bin();
                while (datastorage.readw_bin()) {
                    cs.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cs.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cs.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_grd) {
                datastorage.load_file_metadate_txt();
                while (datastorage.readw_txt()) {
                    cs.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cs.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cs.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                return;
            }
        } else if (ui->comboBox_output_data_params->currentText() == ParametrsList::YongStars_type) {  // Молодые звезды
            Converter_YoungStars cys(datastorage, c, Nbxy);
            cys.set_limits(x, y, z);
            cys.setup_output_data(ui->comboBox_Z->currentText().toStdString());
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_grd) {
                datastorage.load_file_metadate_bin();
                while (datastorage.readw_bin()) {
                    cys.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cys.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cys.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_grd) {
                datastorage.load_file_metadate_txt();
                while (datastorage.readw_txt()) {
                    cys.convert();
                    if (ui->comboBox->currentText() == ParametrsList::is_bin_grd) {
                        cys.save_grd_bin();
                    } else if (ui->comboBox->currentText() == ParametrsList::is_txt_grd) {
                        cys.save_grd_txt();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                        return;
                    }
                }
            } else {
                QMessageBox::warning(this, tr("Ошибка"), tr("указан не существующий тип файла"));
                return;
            }
        }
        QMessageBox::information(this, tr("Уведомление"), tr("Конвертация файлов завершена"));
    } catch (const std::exception &e) {
        QMessageBox::critical(this, tr("Ошибка"), QString("Конвертация не удалась: %1").arg(e.what()));
    }
}
