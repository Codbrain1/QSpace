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
    ui->lineEdit_min_x->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси X"));
        return false;
    }
    ui->lineEdit_max_x->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси X"));
        return false;
    }
    // считываем ограничения для Y
    ui->lineEdit_min_y->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести минимальное значение по оси Y"));
        return false;
    }
    ui->lineEdit_max_y->text().toDouble(&ok);
    if (!ok) {
        statusBar()->setStyleSheet("QStatusBar { color: red; }");
        statusBar()->showMessage(tr("нужно ввести максимальное значение по оси Y"));
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