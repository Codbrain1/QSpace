#pragma once
#include "Core/AppCore/AppCore.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Enums/CoreEnums.h"
#include "Structures/CoreStructures.h"
#include <QObject>
#include <QWidget>
#include <functional>
#include <memory>
#include <qcontainerfwd.h>
#include <qlist.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtreewidget.h>
#include <quuid.h>
#include <qwidget.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class LayerExplorerWidget;
}
QT_END_NAMESPACE

namespace QSpace::UI {

enum TreeDataRole {
    IdRole = Qt::UserRole, // Для хранения QUuid ноды
    TimestampRole,         // Для хранения времени (qint64 или QDateTime)
    DefaultOrderRole       // Для хранения порядкового номера добавления (int)
};

class LayerExplorerWidget : public QWidget {
    Q_OBJECT
  public:
    explicit LayerExplorerWidget(Core::AppCore* m_app, QWidget* parent = nullptr);
    ~LayerExplorerWidget();
    QList<QUuid> getSelectedIds() const;

  signals:
    void selectionChanged(const QList<QUuid>& selectedIds);
    void removalRequested(const QUuid& id);
    void updateNodeSettingsRequested(const QUuid& id, std::function<void(Core::VisualSettings&)> modifer);

    // ------ отвечает за отображение меню с настройками ------
    void propertyInspectorVisibleRequested(const bool isVisible);

  public slots:
    void onNodeAdded(std::shared_ptr<QSpace::Core::DataNode> node);
    void onObjectRemoved(const QUuid& id);

  private slots:

    // общие кнопки
    void on_comboBox_fileStructure_changed(int index);
    void on_comboBox_structureView_changed(int index);
    // ---------------------------------------------------------
    // @SECTION: Редактор слоев
    // ---------------------------------------------------------

    // поиск по слоям
    void on_QLineEdit_findLayer_changed(const QString& line);

    // ------ сортировка слоев ------
    // true -- прямой порядок A - Я, A - Z; false -- обратный порядок Я - A , Z - A
    void on_sortByAlphabetically(const bool direct);
    void on_sortByTimestemp(const bool direct);
    void on_resetSortToDefault();

    //  ------ добавление слоев в редактор слоев ------
    // порождают ноду/контейнер данных в ObjectRegistry
    void on_actionAddLayer_clicked();      // добавление слоя (представления)
    void on_actionAddSnapshot_clicked();   // добавление группы для слоев
    void on_actionAddExperiment_clicked(); // добавление эксперимента

    void on_pushButton_removeElement_clicked(); // удаляет объект в меню

    // ------ управление видимостью слоев ------
    void on_pushButton_hideAll_clicked();        // скрывает все слои
    void on_pushButton_showAll_clicked();        // отображает все слои
    void on_pushButton_hideSelected_clicked();   // скрывает только выбранные слои
    void on_pushButton_showSelected_clicked();   // отображает только выбранные слои
    void on_pushButton_hideUnselected_clicked(); // скрывает невыбранные слои

    // ------ управление фильтрами слоев ------
    void on_checkBox_showLoadedLayers_changed(const bool isChecked);          // только загруженные слои
    void on_checkBox_showUnLoadedLayers_changed(const bool isChecked);        // только незагруженные слои
    void on_checkBox_FilterEquation_changed(const bool isChecked);            // фильтр по выражению
    void on_pushButton_changeFilterEquationLine_clicked(const QString& line); // изменить выражение фильтра

    // ------ урпавление структурой слоев ------
    void on_pushButton_expandAll_clicked();   // разверныть все
    void on_pushButton_collapseAll_clicked(); // скрыть все

    // ---------------------------------------------------------
    // @SECTION: Проводник файлов
    // ---------------------------------------------------------

    // ------ управление корневой директорией ------
    void on_pushButton_changeRootPath_clicked();
    void on_QLineEdit_rootPath_changed(const QString& line);

    //  ------ Добавление/удаление данных ------
    void on_pushButton_importData_clicked(); // загружает данные в ОЗУ
    void on_pushButton_removeData_clicked(); // удаляет данные из ОЗУ
    void on_pushButton_CollapseAllFiles_clicked();

    // поиск по файлам
    void on_QLineEdit_findFile_changed(const QString& line);

    // connect(ui->btn_add_data, &QPushButton::clicked, this, &MainWindow::on_btn_add_data);
    // connect(ui->btn_remove_data, &QPushButton::clicked, this, &MainWindow::on_btn_remove_data);
    void onNodeSelected();
    void showContextMenu(const QPoint& pos);
    void onItemChanged(QTreeWidgetItem* item, int col);
    // void onActionOpenVisualSettingsTrigered();

  private:
    void                     setupSlots();
    void                     setupToolButtons();
    void                     sortTreeHierarchy(QTreeView*                                                  tree,
                                               std::function<bool(const QModelIndex&, const QModelIndex&)> comparator);
    Ui::LayerExplorerWidget* ui;
    Core::AppCore*           m_app;
    Models::DataTreeModel*   m_treeModel = nullptr;

    QString          m_root_path; // по умолчанию равен корню диска
    QTreeWidgetItem* findTreeElementById(const QUuid& id);

    Core::ModelingProgrammVersion m_currentVersion = Core::ModelingProgrammVersion::V2;
    void                          setupFileExplorer();

    void addComponent(); // добавление контейнера над файлами
    void addSnapshot();  // добавление снимка
};
} // namespace QSpace::UI