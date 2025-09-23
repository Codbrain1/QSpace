#include "mainwindow.h"

#include <qcontainerfwd.h>
#include <qdir.h>

#include <QFileDialog>
#include <QFormLayout>
#include <QMessageBox>
#include <cstddef>

#include "./ui_mainwindow.h"
#include "Converter/Converter.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_txt_clicked()
{
    try {
        QStringList inputfileNames =
            QFileDialog::getOpenFileNames(this, tr("Select TXT Files"), "", tr("Text Files (*.txt)"));

        if (!inputfileNames.isEmpty()) {
            inputFiles = inputfileNames;
            // добавляем текст входного файла в input_line_edit
            ui->lineEdit_input->setText(inputfileNames.first() + ", ...");

            // если выходной путь не выбран автоматически выбирается корневая директория входных файлов/output
            if (ui->lineEdit_output->text().toStdString().empty()) {
                QDir current_dir = QDir::currentPath();
                QString outputfileName = "output";
                QString new_output_folder_name = current_dir.absolutePath() + "/" + outputfileName;
                outputDir = new_output_folder_name;
                ui->lineEdit_output->setText(outputDir);
            }

            // Проверяем ui->scrollContent
            if (!ui->widget_2) {
                qDebug() << "Error: ui->scrollContent is null!";
                QMessageBox::critical(this, tr("Ошибка"), tr("ui->scrollContent не инициализирован!"));
                return;
            }

            // Проверяем, есть ли layout_columns
            QLayout *layout = ui->formLayout_columns;
            if (!layout) {
                qDebug() << "Error: ui->scrollContent has no layout!";
                QMessageBox::critical(this, tr("Ошибка"), tr("ui->scrollContent не имеет layout!"));
                return;
            }

            QFormLayout *columnsLayout = qobject_cast<QFormLayout *>(layout);
            if (!columnsLayout) {
                qDebug() << "Error: Layout is not QFormLayout, actual type:" << layout->metaObject()->className();
                std::string ss = layout->metaObject()->className();
                std::string ss1 = "Layout не является QFormLayout! актуальный тип:";
                ss1 += ss;
                QMessageBox::critical(this, tr("Ошибка"), tr(ss1.c_str()));
                return;
            }

            QLayoutItem *item;
            while ((item = columnsLayout->takeAt(0)) != nullptr) {
                if (QWidget *widget = item->widget()) {
                    widget->deleteLater();  // Безопасное удаление виджета
                }
                delete item;  // Удаляем QLayoutItem
            }

            QString first_file = inputfileNames.first();
            if (!first_file.isEmpty()) {
                Converter<double> first_converter;
#ifdef _WIN32
                first_translator.load_input_file(std::filesystem::path(first_file.toStdU16String()));
#else
                first_converter.load_input_file(std::filesystem::path(first_file.toStdString()));
#endif
                // Получите число колонок
                int numColumns = first_converter.get_column_count();

                // елси файл не содержит колонок
                if (numColumns <= 0) {
                    std::string file_path = first_file.toStdString();
                    std::string text = "Файл пуст или не содержит колонок: ";
                    QMessageBox::warning(this, tr("Ошибка"), tr(std::string(text + file_path).c_str()));
                    return;
                }
                if (numColumns != columnCombos.size()) {
                    columnCombos.clear();
                    auto optionsStd = Converter<double>::get_columns();
                    QStringList options;
                    for (const auto &opt : optionsStd) {
                        options.append(QString::fromStdString(std::string(opt)));
                    }

                    for (int i = 0; i < numColumns; ++i) {
                        QLabel *label = new QLabel(tr("Колонка %1:").arg(i + 1), this);
                        QComboBox *combo = new QComboBox(this);
                        combo->addItems(options);
                        combo->setCurrentIndex(options.indexOf("None"));  // По умолчанию "None"
                        columnsLayout->addRow(label, combo);
                        columnCombos.append(combo);
                    }
                }

                first_converter.clear();
                for (auto &inputfileName : inputfileNames) {
                    // если  входной путь не пустой
                    if (!inputfileName.isEmpty()) {
                        Converter<double> item_converter;
#ifdef _WIN32
                        item_translator.load_input_file(std::filesystem::path(inputfileName.toStdU16String()));
#else
                        item_converter.load_input_file(std::filesystem::path(inputfileName.toStdString()));
#endif
                        // Получите число колонок
                        int numColumns_i = item_converter.get_column_count();

                        // елси файл не содержит колонок
                        if (numColumns_i <= 0) {
                            std::string file_path = inputfileName.toStdString();
                            std::string text = "Файл пуст или не содержит колонок: ";
                            QMessageBox::warning(this, tr("Ошибка"), tr(std::string(text + file_path).c_str()));
                            return;
                        }
                        if (numColumns_i != numColumns) {
                            std::string file_path = inputfileName.toStdString();
                            std::string first_file_path = first_file.toStdString();
                            std::string text = "число колонок в файлах не совпадают: ";
                            QMessageBox::warning(this, tr("Ошибка"),
                                                 tr(std::string(text + file_path + " != " + first_file_path).c_str()));
                            return;
                        }
                    }
                }
            }
        }
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
    try {
        QDir dir;
        if (!dir.exists(outputDir)) {
            if (!dir.mkpath(outputDir)) {
                QMessageBox::warning(this, "Ошибка", "Не удалось создать папку. Проверь права доступа.");
            }
        }
        for (auto &inputfileName : inputFiles) {
            QFileInfo fileInfo(inputfileName);                                                // Извлекаем информацию о файле
            QString outputfileName = outputDir + "/" + fileInfo.completeBaseName() + ".grd";  // Имя без расширения + .grd
            converters.push_back(Converter<double>());
            auto &item_converter = converters.back();
#ifdef _WIN32
            item_converter.load_input_file(std::filesystem::path(inputfileName.toStdU16String()));
            item_converter.load_output_file(outputfileName.toStdU16String());
#else
            item_converter.load_input_file(std::filesystem::path(inputfileName.toStdString()));
            item_converter.load_output_file(outputfileName.toStdString());
#endif
        }
    } catch (const std::exception &e) {
        QMessageBox::critical(this, tr("Ошибка"), QString("Ошибка открытия выходного файла: %1").arg(e.what()));
    }

    std::vector<std::string> columnMappings;  // вектор колонок
    std::vector<std::string> xyzNames;        // вектор координат в grd файле
    for (auto &combo : columnCombos) {
        QString value = combo->currentText();
        columnMappings.push_back(value.toStdString());
    }
    QString xyz_value = ui->comboBox_X->currentText().replace(" (координата)", "");
    xyzNames.push_back(xyz_value.toStdString());
    xyz_value = ui->comboBox_Y->currentText().replace(" (координата)", "");
    xyzNames.push_back(xyz_value.toStdString());
    xyz_value = ui->comboBox_Z->currentText();
    if (xyz_value.toStdString() == "density (плотность)")
        xyzNames.push_back("density");
    else if (xyz_value.toStdString() == "Termal (температура)")
        xyzNames.push_back("Termal");

    if (columnMappings.empty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не выбраны колонки для маппинга."));
        return;
    }
    if (xyzNames.size() != 3) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Должны быть выбраны x, y, z."));
        return;
    }

    // Получите лимиты
    bool ok;
    double xMin = ui->lineEdit_min_x->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный X Min"));
        return;
    }
    double xMax = ui->lineEdit_max_x->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный X Max"));
        return;
    }
    double yMin = ui->lineEdit_min_y->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Y Min"));
        return;
    }
    double yMax = ui->lineEdit_max_y->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Y Max"));
        return;
    }
    // double zMin = ui->zMinLineEdit->text().toDouble(&ok);
    // if (!ok) { QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Z Min")); return; }
    // double zMax = ui->zMaxLineEdit->text().toDouble(&ok);
    // if (!ok) { QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Z Max")); return; }
    int nx = ui->lineEdit_Nx->text().toInt(&ok);
    if (!ok || nx <= 0) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Nx"));
        return;
    }
    int ny = ui->lineEdit_Ny->text().toInt(&ok);
    if (!ok || ny <= 0) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Некорректный Ny"));
        return;
    }

    try {
        for (size_t i = 0; i < converters.size(); ++i) {
            // Настроука Converter
            converters[i].setup_columns(columnMappings, xyzNames);
            converters[i].setup_gridXYZ({xMin, xMax}, {yMin, yMax}, {0, 1}, {nx, ny});
            converters[i].read_input_file();
            converters[i].translate_to_grd_bindings();
            converters[i].clear_output_file_state();
        }
        QMessageBox::information(this, tr("Успех"), tr("Конвертация завершена."));
    } catch (const std::exception &e) {
        QMessageBox::critical(this, tr("Ошибка"), QString("Конвертация не удалась: %1").arg(e.what()));
    }
}
