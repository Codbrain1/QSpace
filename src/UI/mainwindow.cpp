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
#include <memory>

#include "./ui_mainwindow.h"

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
    connect(ui->lineEdit_hb, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--x min--*/ connect(ui->lineEdit_min_x, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--x max--*/ connect(ui->lineEdit_max_x, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--y min--*/ connect(ui->lineEdit_min_y, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--x max--*/ connect(ui->lineEdit_max_y, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--z min--*/ connect(ui->lineEdit_min_z, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);
    /*--z max--*/ connect(ui->lineEdit_max_z, &QLineEdit::textChanged, this, &MainWindow::on_lineEdit_Slice_changed);

    on_comboBox_output_data_params_changed(ui->comboBox_output_data_params->currentIndex());  // устанавливаем начальное
                                                                                              // состояние выходных
    on_lineEdit_Slice_changed("");                                                            // параметров
}
