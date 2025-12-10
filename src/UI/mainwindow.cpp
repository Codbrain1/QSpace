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
#include <QtDataVisualization/Q3DScatter>
#include <QtDataVisualization/QScatter3DSeries>
#include <random>
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);  // установка начальных настроек интерфейса

    inputfiles_names.clear();
    outputDir.clear();
    loadSettings();
//     // init3D();
//     // 1. Создаём график
// auto *scatter = new Q3DScatter();

// // 2. ВСТАВЛЯЕМ ЕГО ПРЯМО В ТВОЙ УЖЕ СУЩЕСТВУЮЩИЙ widget_graphic ИЗ .UI
// ui->widget_graphic->setLayout(new QVBoxLayout(ui->widget_graphic));
// ui->widget_graphic->layout()->setContentsMargins(0,0,0,0);
// ui->widget_graphic->layout()->addWidget(QWidget::createWindowContainer(scatter));

// // 3. Оптимизация под 1 000 000 частиц
// scatter->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
// scatter->setOptimizationHints(QAbstract3DGraph::OptimizationStatic);

// auto *series = new QScatter3DSeries;
// series->setItemSize(0.0f);        // пиксельные точки = максимум FPS
// series->setMesh(QAbstract3DSeries::MeshPoint);
// scatter->addSeries(series);

// // Пример: сразу закидываем 900 000 частиц
// QScatterDataArray data;
// data.resize(900'000);
// // Быстрая генерация (сфера)
//   std::mt19937 gen(std::random_device{}());
//   std::uniform_real_distribution<float> dist(-8.0f, 8.0f);

//   for (int i = 0; i < 200'000; ++i) {
//     float r = dist(gen) * 0.1f;
//     float theta = dist(gen) * 6.28318f;
//     float phi = std::acos(1.0f - 2.0f * (dist(gen) + 8.0f) / 16.0f);
//     float x = r * std::sin(phi) * std::cos(theta);
//     float y = r * std::sin(phi) * std::sin(theta);
//     float z = r * std::cos(phi);
//     data[i].setPosition(QVector3D(x * 5, y * 5, z * 5));
//   }

// series->dataProxy()->resetArray(&data);   // ← мгновенно!
    connectSlots();
}
