#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <q3dscatter.h>
#include <qcontainerfwd.h>
#include <qcoreevent.h>
#include <qlistwidget.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpoint.h>
#include <qscatter3dseries.h>
#include <qscatterdataproxy.h>

#include <QComboBox>
#include <QMainWindow>
#include <QtDataVisualization/Q3DScatter>
#include <QtDataVisualization/QScatter3DSeries>
#include <cstddef>
#include <memory>
#include <vector>

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
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

   private slots:
    void on_comboBox_output_data_params_changed(int index);
    void on_lineEdit_Slice_changed(const QString&);
    void on_pushButton_input_files_clicked();

    void on_pushButtongrd_clicked();
    void on_listWidget_right_mouse_clicked(const QPoint& pos);
    bool eventFilter(QObject* obj, QEvent* event) override;
    void on_pushButtonconvert_clicked();
    void on_lineEdit_numbers_columns_files_changed(const QString& changed_line);
    void on_checkbox_column_customise_change(Qt::CheckState state);
    void on_lineEdit_projection_area_changed(const QString&);  // все QLineEdit отвечающие за изменение слоя отображения
    void on_pushButton_add_input_file_clicked();
    void on_action_reset_settings();
    // void on_list_widget_input_files_item_clicked(QListWidgetItem* item);

   protected:
    void closeEvent(QCloseEvent* event) override;

   private:
    // -----------------------------------------------------поля-----------------------------------------------------
    std::unique_ptr<Ui::MainWindow> ui;  // указатель на основное окно приложения
    QStringList inputfiles_names;
    QString outputDir;  // имя выходной директории
    QVector<QComboBox*> columnCombos;
    // -----------------------------------------------------методы-----------------------------------------------------
    void set_False_Enable_Line_Edit_Constants();
    void setup_columns_comboBoxes_DM_S(const int num_col);
    void setup_columns_comboBoxes_G_MC_YS(const int num_col);
    // void setup_columns_comboBoxes(const int num_col);
    bool is_correct_data_QLineEdit();
    void delete_selected_files();
    void loadSettings();
    void connectSlots();
    // void DrawFile(std::vector<double>& x, std::vector<double>& y, std::vector<double>& z, std::vector<double>& value,
    //               int Ns_i, int offset);
    // void init3D();
};
#endif  // MAINWINDOW_H
