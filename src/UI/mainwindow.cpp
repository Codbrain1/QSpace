#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QMessageBox>
#include<iostream>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
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
        QString inputfileName = QFileDialog::getOpenFileName(this, tr("Select TXT File"), "", tr("Text Files (*.txt)"));
        // если входная входной путь не пустой
        if (!inputfileName.isEmpty()) {
            // добавляем текст входного файла в input_line_edit
            ui->lineEdit_input->setText(inputfileName);

            // если выходной путь не выбран автоматически выбирается имя входного файла с расширением txt
            if (ui->lineEdit_output->text().toStdString().empty()) {
                QFileInfo fileInfo(inputfileName);  // Извлекаем информацию о файле
                QString outputfileName =
                    fileInfo.path() + "/" + fileInfo.completeBaseName() + ".grd";  // Имя без расширения + .grd
                ui->lineEdit_output->setText(outputfileName);
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

#ifdef _WIN32
            translator.load_input_file(std::filesystem::path(inputfileName.toStdU16String()));
#else
            translator.load_input_file(std::filesystem::path(inputfileName.toStdString()));
#endif
            // Получите число колонок
            int numColumns = translator.get_column_count();

            // елси файл не содержит колонок
            if (numColumns <= 0) {
                QMessageBox::warning(this, tr("Ошибка"), tr("Файл пуст или не содержит колонок."));
                return;
            }
            if (numColumns != columnCombos.size()) {
                columnCombos.clear();
                auto optionsStd = translator.get_columns();
                QStringList options;
                for (const auto &opt : optionsStd) {
                    options.append(QString::fromStdString(opt));
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
        }
    } catch (const char *ex) {
        QMessageBox::warning(this, tr("Ошибка"), tr(ex));
        return;
    }
}


void MainWindow::on_pushButtongrd_clicked()
{
    QString outputfileName = QFileDialog::getSaveFileName(this, tr("Select GRD File"), "", tr("GRD Files (*.grd)"));
    if (!outputfileName.isEmpty()) {
        ui->lineEdit_output->setText(outputfileName);
        try {
#ifdef _WIN32
            translator.load_output_file(outputfileName.toStdU16String());
#else
            translator.load_output_file(outputfileName.toStdString());
#endif
        } catch (const std::exception &e) {
            QMessageBox::critical(this, tr("Ошибка"), QString("Ошибка открытия выходного файла: %1").arg(e.what()));
        }
    }

}


void MainWindow::on_pushButtonconvert_clicked()
{
    QString input = ui->lineEdit_input->text();
    QString output = ui->lineEdit_output->text();
    // если входной или выходной пути пусты
    if (input.isEmpty() || output.isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Выберите файлы ввода и вывода."));
        return;
    }
    try {
#ifdef _WIN32
        translator.load_output_file(output.toStdU16String());
#else
        translator.load_output_file(output.toStdString());
#endif
    } catch (const std::exception &e) {
        QMessageBox::critical(this, tr("Ошибка"), QString("Ошибка открытия выходного файла: %1").arg(e.what()));
        return;
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
        // Настройте Translator
        translator.setup_columns(columnMappings, xyzNames);
        translator.setup_gridXYZ({xMin, xMax}, {yMin, yMax}, {0, 1}, {nx, ny});
        translator.read_file();
        translator.translate_to_grd_bindings();
        QMessageBox::information(this, tr("Успех"), tr("Конвертация завершена."));
        translator.clear_output_file_state();
    } catch (const std::exception &e) {
        QMessageBox::critical(this, tr("Ошибка"), QString("Конвертация не удалась: %1").arg(e.what()));
    }
}

