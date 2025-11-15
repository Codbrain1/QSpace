#include "mainwindow.h"

#include <qaction.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qkeysequence.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpaintdevice.h>
#include <qpushbutton.h>

#include <QFileDialog>
#include <QFormLayout>
#include <QMessageBox>
#include <QSettings>
#include <memory>

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);  // установка начальных настроек интерфейса

    inputfiles_names.clear();
    outputDir.clear();
    loadSettings();
    // init3D();
    connectSlots();
}
