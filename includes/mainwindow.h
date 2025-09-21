#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QMainWindow>

#include "Converter/Converter.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

   public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   private slots:
    void on_pushButton_txt_clicked();

    void on_pushButtongrd_clicked();

    void on_pushButtonconvert_clicked();

   private:
    Ui::MainWindow *ui;
    Translator<double> translator;
    QVector<QComboBox *> columnCombos;
};
#endif  // MAINWINDOW_H
