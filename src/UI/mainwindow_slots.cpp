#include <qcompare.h>
#include <qcoreevent.h>
#include <qlistwidget.h>
#include <qmainwindow.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qobject.h>
#include <qpoint.h>
#include <qsettings.h>
#include <qwidget.h>

#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <algorithm>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "./ui_mainwindow.h"
#include "ConstantsParametrs.h"
#include "Converter/Converter_DarkMatter.h"
#include "Converter/Converter_Gas.h"
#include "Converter/Converter_MolecularClouds.h"
#include "Converter/Converter_Stars.h"
#include "Converter/Converter_YoungStars.h"
#include "mainwindow.h"

// отвечает за установку проецируемой области
void MainWindow::on_lineEdit_projection_area_changed(const QString&)
{
    statusBar()->clearMessage();
    statusBar()->setStyleSheet("");

    if (is_correct_data_QLineEdit()) {
        double hb = ui->lineEdit_hb->text().toDouble();
        double xmin = ui->lineEdit_min_x_lim->text().toDouble();
        double xmax = ui->lineEdit_max_x_lim->text().toDouble();
        double ymin = ui->lineEdit_min_y_lim->text().toDouble();
        double ymax = ui->lineEdit_max_y_lim->text().toDouble();
        // вычисляем точки
        int Nbx = static_cast<int>((xmax - xmin + hb) / hb + 0.5);
        int Nby = static_cast<int>((ymax - ymin + hb) / hb + 0.5);
        ui->lineEdit_Nx->setText(std::to_string(Nbx).c_str());
        ui->lineEdit_Ny->setText(std::to_string(Nby).c_str());
    }
}
MainWindow::~MainWindow() {}

// задает разрешенные выходные параметры для записи, устанавливает разрешенные константы изменяемыми, задает число колонок по
// умолчанию в файле
void MainWindow::on_comboBox_output_data_params_changed(int index)  // управляет списком выходных записываемых параметров
{
    QString curent_param = ui->comboBox_output_data_params->itemText(index);  // получаем значение типа выходных данных
    ui->comboBox_Z->clear();                                                  // очищаем список выбора типа выходных данных
    for (const auto& item : ParametrsList::Z_outParams) {  // цикл по списку разрешенных параметров записи в файл
        ui->comboBox_Z->addItem(QString(item.data()));
    }
    ui->comboBox_Z->addItem(QString(ParametrsList::Z_outParamKit.data()));  // пользовательский набор записываемых выходных
                                                                            // параметров
    const auto& PL_NoTermal = ParametrsList::NoTermal_odata_t;
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
            setup_columns_comboBoxes_DM_S(7);
        }
    } else if (curent_param == ParametrsList::Gas_type || curent_param == ParametrsList::YongStars_type ||
               curent_param == ParametrsList::MolecularClouds_type) {
        ui->lineEdit_gamma->setEnabled(true);
        ui->lineEdit_Km->setEnabled(true);
        ui->lineEdit_Kr->setEnabled(true);
        if (!ui->checkBox_column_customize->checkState()) {
            ui->lineEdit_numbers_columns_ifiles->setText("11");
            setup_columns_comboBoxes_G_MC_YS(11);
        }
    }
}
// проверяем что записано разрешенное число колонок в lineEdit_numbers_columns_files
void MainWindow::on_lineEdit_numbers_columns_files_changed(const QString& changed_line)
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
        QString curent_param = ui->comboBox_output_data_params->currentText();
        // разрешение на редактирование констант и установка числа колонок в файлах
        if (curent_param == ParametrsList::DarkMatter_type || curent_param == ParametrsList::Stars_type) {
            setup_columns_comboBoxes_DM_S(column_size);
        } else if (curent_param == ParametrsList::Gas_type || curent_param == ParametrsList::YongStars_type ||
                   curent_param == ParametrsList::MolecularClouds_type) {
            setup_columns_comboBoxes_G_MC_YS(column_size);
        }
    }
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
// слот выбора входных файлов (удаляет предыдущие)
void MainWindow::on_pushButton_input_files_clicked()
{
    try {
        std::string filter = "";
        if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_ifiles) {
            filter = "(*.txt)";
        } else {
            filter = "(*.bin)";
        }
        QStringList inputfileNames =
            QFileDialog::getOpenFileNames(this, tr("Select TXT Files"), "", tr(("Text Files " + filter).c_str()));

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
        for (const auto& file_name : inputfileNames) {
            QFileInfo file;
            if (!file.exists(file_name)) {
                QMessageBox::warning(this, tr("Ошибка"), tr("Файл не существует"));
                return;
            }
        }
        QFileInfo file(inputfileNames.back());
        QString parentDir = QDir::toNativeSeparators(file.absolutePath());
        ui->lineEdit_input->setText(parentDir);

        ui->listWidget_input_file_names->clear();
        ui->listWidget_input_files->clear();
        for (auto file_name : inputfileNames) {
            ui->listWidget_input_file_names->addItem(file_name);
            ui->listWidget_input_files->addItem(file_name);
        }
        inputfiles_names.clear();
        inputfiles_names = inputfileNames;
        inputfileNames.clear();
        inputfileNames.shrink_to_fit();
    } catch (const char* ex) {
        QMessageBox::warning(this, tr("Ошибка"), tr(ex));
        return;
    }
}
// слот выбора выходной директории (по умолчанию создает output)
void MainWindow::on_pushButtongrd_clicked()
{
    QString outputfolderName = QFileDialog::getExistingDirectory(nullptr,  // Родительский виджет (nullptr для модального
                                                                           // окна)
                                                                 tr("Выберите или создайте папку"),  // Заголовок окна
                                                                 QDir::currentPath(),  // Начальная директория (например,
                                                                 QFileDialog::ShowDirsOnly  // Показывать только папки
    );
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
// конвертация файлов в grd
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
        for (const auto& ifile_name : inputfiles_names) {
            ifiles_names.push_back(ifile_name.toStdString());
        }

        //----------------------------------------------------------------------------
        DataStorage datastorage(ifiles_names);  // создание хранилища данных

        // преобразование combobox в vector<int> для колонок
        std::vector<std::string> columns;
        columns.reserve(columnCombos.size());
        for (const auto& col : columnCombos) {
            columns.push_back(col->currentText().toStdString());
        }

        datastorage.setup_columns(columns);  // задаем колонки в входных файлах
        datastorage.set_buff_size(8);        // устанавливаем число файлов записываемых за раз на диск
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

        lim<double> x_lim;
        lim<double> y_lim;
        x_lim.max = ui->lineEdit_max_x_lim->text().toDouble();
        x_lim.min = ui->lineEdit_min_x_lim->text().toDouble();
        y_lim.max = ui->lineEdit_max_y_lim->text().toDouble();
        y_lim.min = ui->lineEdit_min_y_lim->text().toDouble();
        //---------------------задание проекции на сетку-------------------
        double angle_teta = ui->lineEdit_angle_teta->text().toDouble();
        double angle_psi = ui->lineEdit_angle_psi->text().toDouble();

        //------------задание области ограничения---------------------------

        lim<double> x_bound;
        lim<double> y_bound;
        lim<double> z_bound;
        x_bound.max = ui->lineEdit_max_x_bound->text().toDouble();
        x_bound.min = ui->lineEdit_min_x_bound->text().toDouble();
        y_bound.max = ui->lineEdit_max_y_bound->text().toDouble();
        y_bound.min = ui->lineEdit_min_y_bound->text().toDouble();
        z_bound.max = ui->lineEdit_max_z_bound->text().toDouble();
        z_bound.min = ui->lineEdit_min_z_bound->text().toDouble();

        //------------задание углов области ограничения---------------------------
        double angle_alpha = ui->lineEdit_angle_alpha->text().toDouble();
        double angle_beta = ui->lineEdit_angle_beta->text().toDouble();
        double angle_phi = ui->lineEdit_angle_phi->text().toDouble();

        auto X = ui->comboBox_X->currentText().toStdString();
        auto Y = ui->comboBox_Y->currentText().toStdString();
        const std::pair<std::string, std::string> XY = {X, Y};
        std::vector<std::string> Z_col_list;
        Z_col_list.push_back(ui->comboBox_Z->currentText().toStdString());
        //------------------------------создание конвертера------------------------------------------
        if (ui->comboBox_output_data_params->currentText() == ParametrsList::DarkMatter_type) {  // Темная Материя
            Converter_DarkMatter cdm(datastorage, c, Nbxy, outputDir.toStdString());
            cdm.set_limits(x_lim, y_lim, angle_teta, angle_psi);
            cdm.set_boundary(x_bound, y_bound, z_bound, angle_alpha, angle_beta, angle_phi);

            cdm.setup_output_data(Z_col_list, XY);
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_ifiles) {  // если файлы бинарные
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
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_ifiles) {
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
            Converter_Gas cg(datastorage, c, Nbxy, outputDir.toStdString());
            cg.set_limits(x_lim, y_lim, angle_teta, angle_psi);
            cg.set_boundary(x_bound, y_bound, z_bound, angle_alpha, angle_beta, angle_phi);

            cg.setup_output_data(Z_col_list, XY);
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_ifiles) {
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
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_ifiles) {
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
            Converter_MolecularClouds cmc(datastorage, c, Nbxy, outputDir.toStdString());
            cmc.set_limits(x_lim, y_lim, angle_teta, angle_psi);
            cmc.set_boundary(x_bound, y_bound, z_bound, angle_alpha, angle_beta, angle_phi);

            cmc.setup_output_data(Z_col_list, XY);
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_ifiles) {
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
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_ifiles) {
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
            Converter_Stars cs(datastorage, c, Nbxy, outputDir.toStdString());
            cs.set_limits(x_lim, y_lim, angle_teta, angle_psi);
            cs.set_boundary(x_bound, y_bound, z_bound, angle_alpha, angle_beta, angle_phi);
            cs.setup_output_data(Z_col_list, XY);
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_ifiles) {
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
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_ifiles) {
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
            Converter_YoungStars cys(datastorage, c, Nbxy, outputDir.toStdString());
            cys.set_limits(x_lim, y_lim, angle_teta, angle_psi);
            cys.set_boundary(x_bound, y_bound, z_bound, angle_alpha, angle_beta, angle_phi);

            cys.setup_output_data(Z_col_list, XY);
            if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_ifiles) {
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
            } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_ifiles) {
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
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Ошибка"), QString("Конвертация не удалась: %1").arg(e.what()));
    }
}
// обработка кнопки delete для удаления файлов которые были выбраны
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if ((obj == ui->listWidget_input_file_names || obj == ui->listWidget_input_files) && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace) {
            delete_selected_files();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
// обработка нажатия правой кнопки мыши
void MainWindow::on_listWidget_right_mouse_clicked(const QPoint& pos)
{
    QListWidget* clickedWidget = qobject_cast<QListWidget*>(sender());
    if (!clickedWidget) return;

    QMenu menu;
    menu.addAction("Удалить", this, &MainWindow::delete_selected_files);
    menu.exec(clickedWidget->mapToGlobal(pos));
}
// добавление входных файлов
void MainWindow::on_pushButton_add_input_file_clicked()
{
    try {
        std::string filter = "";
        if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_ifiles) {
            filter = "(*.txt)";
        } else {
            filter = "(*.bin)";
        }
        QStringList inputfileNames =
            QFileDialog::getOpenFileNames(this, tr("Select TXT Files"), "", tr(("Text Files " + filter).c_str()));

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
        for (const auto& file_name : inputfileNames) {
            QFileInfo file;
            if (!file.exists(file_name)) {
                QMessageBox::warning(this, tr("Ошибка"), tr("Файл не существует"));
                return;
            }
        }

        QFileInfo file(inputfileNames.back());
        QString parentDir = QDir::toNativeSeparators(file.absolutePath());
        ui->lineEdit_input->setText(parentDir);

        for (auto file_name : inputfileNames) {
            ui->listWidget_input_file_names->addItem(file_name);
            ui->listWidget_input_files->addItem(file_name);
        }

        for (auto i : inputfileNames) {
            inputfiles_names.push_back(i);
        }
    } catch (const char* ex) {
        QMessageBox::warning(this, tr("Ошибка"), tr(ex));
        return;
    }
}
void MainWindow::on_action_reset_settings()
{
    if (QMessageBox::question(this, tr("Сброс"), tr("Сбросить все настройки и перезапустить приложение?")) ==
        QMessageBox::Yes) {
        QSettings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat).clear();
        loadSettings();
    }
}
void MainWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);

    // === 1. Окно ===
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());
    // === 2. Вкладки ===
    settings.setValue("tabs/current", ui->tabWidget->currentIndex());

    // === 3. Пути ===
    settings.setValue("files/input", inputfiles_names);
    settings.setValue("files/output_dir", outputDir);

    // === 4. ComboBox'ы ===
    settings.setValue("combos/output_type", ui->comboBox->currentIndex());
    settings.setValue("combos/data_type", ui->comboBox_output_data_params->currentIndex());
    settings.setValue("combos/X", ui->comboBox_X->currentIndex());
    settings.setValue("combos/Y", ui->comboBox_Y->currentIndex());
    settings.setValue("combos/Z", ui->comboBox_Z->currentIndex());
    settings.setValue("combos/file_structure", ui->comboBox_type_structures_ifiles->currentIndex());

    // === 5. Параметры сетки ===
    settings.setValue("boundary/min_x", ui->lineEdit_min_x_bound->text());
    settings.setValue("boundary/max_x", ui->lineEdit_max_x_bound->text());
    settings.setValue("boundary/min_y", ui->lineEdit_min_y_bound->text());
    settings.setValue("boundary/max_y", ui->lineEdit_max_y_bound->text());
    settings.setValue("boundary/min_z", ui->lineEdit_min_z_bound->text());
    settings.setValue("boundary/max_z", ui->lineEdit_max_z_bound->text());
    settings.setValue("boundary/alpha", ui->lineEdit_angle_alpha->text());
    settings.setValue("boundary/beta", ui->lineEdit_angle_beta->text());
    settings.setValue("boundary/phi", ui->lineEdit_angle_phi->text());

    // === 5. Параметры сетки ===
    settings.setValue("grid/min_x", ui->lineEdit_min_x_lim->text());
    settings.setValue("grid/max_x", ui->lineEdit_max_x_lim->text());
    settings.setValue("grid/min_y", ui->lineEdit_min_y_lim->text());
    settings.setValue("grid/max_y", ui->lineEdit_max_y_lim->text());
    settings.setValue("grid/teta", ui->lineEdit_angle_teta->text());
    settings.setValue("grid/psi", ui->lineEdit_angle_psi->text());
    settings.setValue("grid/hb", ui->lineEdit_hb->text());

    // === 6. Константы ===
    settings.setValue("const/gamma", ui->lineEdit_gamma->text());
    settings.setValue("const/Km", ui->lineEdit_Km->text());
    settings.setValue("const/Kr", ui->lineEdit_Kr->text());
    settings.setValue("lineedits/input", ui->lineEdit_input->text());
    settings.setValue("lineedits/output", ui->lineEdit_output->text());
    // TODO: добавить все константы из программы

    // === 7. QListWidget ===
    settings.beginWriteArray("files/input");
    for (int i = 0; i < ui->listWidget_input_file_names->count(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("path", ui->listWidget_input_file_names->item(i)->text());
    }
    settings.endArray();
    // === 8. CheckBox ===
    settings.setValue("checkbox/custom_columns", ui->checkBox_column_customize->isChecked());
    event->accept();
}
void MainWindow::on_lineEdit_Slice_changed(const QString&)
{
    is_correct_data_QLineEdit();
}
// void MainWindow::on_list_widget_input_files_item_clicked(QListWidgetItem* item)
// {
//     if (item->text().isEmpty()) {
//         QMessageBox::warning(this, tr("Ошибка"), tr("выбран некорректный файл"));
//         return;
//     }
//     std::filesystem::path file = item->text().toStdString();
//     DataStorage storage({file});
//     // преобразование combobox в vector<int> для колонок
//     std::vector<std::string> columns;
//     columns.reserve(columnCombos.size());
//     for (const auto& col : columnCombos) {
//         columns.push_back(col->currentText().toStdString());
//     }
//     storage.set_buff_size(1);
//     storage.setup_columns(columns);
//     if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_bin_ifiles) {
//         storage.load_file_metadate_bin();
//         storage.readw_bin();
//     } else if (ui->comboBox_type_structures_ifiles->currentText() == ParametrsList::is_txt_ifiles) {
//         storage.load_file_metadate_txt();
//         storage.readw_txt();
//     } else {
//         QMessageBox::warning(this, tr("Ошибка"), tr("выбран неверный тип файлов"));
//     }

//     auto& x = storage.get_x();
//     auto& y = storage.get_y();
//     auto& z = storage.get_z();
//     auto& rho = storage.get_rho();
//     size_t Ns_i = storage.get_Ns()[0];
//     size_t offset = storage.get_offsets()[0];
//     DrawFile(x, y, z, rho, Ns_i, offset);
// }