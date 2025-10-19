#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qcontainerfwd.h>
#include <qnamespace.h>
#include <qobject.h>

#include <QComboBox>
#include <QMainWindow>
#include <memory>

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
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   private slots:
    void on_comboBox_output_data_params_changed(int index);

    void on_pushButton_txt_clicked();

    void on_pushButtongrd_clicked();

    void on_pushButtonconvert_clicked();
    void on_lineEdit_numbers_columns_files_changed(const QString &changed_line);
    void on_checkbox_column_customise_change(Qt::CheckState state);
    void on_lineEdit_hb_changed(const QString &changed_line);

   private:
    // -----------------------------------------------------поля-----------------------------------------------------
    std::unique_ptr<Ui::MainWindow> ui;  // указатель на основное окно приложения
    QStringList inputfiles_names;
    QString outputDir;  // имя выходной директории
    QVector<QComboBox *> columnCombos;

    // -----------------------------------------------------методы-----------------------------------------------------
    void set_False_Enable_Line_Edit_Constants();
    void setup_columns_comboBoxes(const int num_col);
};
#endif  // MAINWINDOW_H
