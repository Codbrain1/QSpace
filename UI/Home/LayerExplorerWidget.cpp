#include "LayerExplorerWidget.h"
#include "Core/AppCore/AppCore.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Enums/CoreEnums.h"
#include "Presentation/DataTreeModel/DataTreeModel.h"
#include "Structures/CoreStructures.h"
#include "ui_LayerExplorerWidget.h"
#include <Core/AppCore/DataController.h>
#include <QActionGroup>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <memory>
#include <qabstractspinbox.h>
#include <qaction.h>
#include <qcontainerfwd.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtreeView.h>
#include <qtreeview.h>
#include <quuid.h>

namespace QSpace::UI {
class CustomSortProxyModel : public QSortFilterProxyModel {
    std::function<bool(const QModelIndex&, const QModelIndex&)> m_comparator;

  public:
    CustomSortProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
    }

    void setComparator(std::function<bool(const QModelIndex&, const QModelIndex&)> comp) {
        m_comparator = comp;
        invalidate(); // Заставляем модель пересортироваться
    }

  protected:
    bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override {
        if (m_comparator) {
            return m_comparator(source_left, source_right);
        }
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
};

LayerExplorerWidget::LayerExplorerWidget(Core::AppCore* app, QWidget* parent)
    : QWidget(parent), ui(new Ui::LayerExplorerWidget), m_app(app) {
    ui->setupUi(this);

    // 1. Подключаем готовую модель из AppCore к нашему QTreeView (ui->treeView_Layers)
    if (m_app && m_app->dataTreeModel()) {
        auto* proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(m_app->dataTreeModel());
        proxyModel->setFilterKeyColumn(0); // Фильтруем по первой колонке (название слоя)
        proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

        // В QTreeView устанавливаем ИМЕННО прокси-модель
        ui->treeView_Layers->setModel(proxyModel);
        // ui->treeView_Layers->setModel(m_app->dataTreeModel());
    }

    // Настройки отображения
    ui->treeView_Layers->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView_Layers->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeView_Layers->setAnimated(true);

    // выбор версии программы моделирвоания для загрузки файлов
    ui->comboBox_fileStructure->setItemData(0, static_cast<int>(Core::ModelingProgrammVersion::V2));
    ui->comboBox_fileStructure->setItemData(1, static_cast<int>(Core::ModelingProgrammVersion::V2_2));
    ui->comboBox_fileStructure->setItemData(2, static_cast<int>(Core::ModelingProgrammVersion::V2_3));

    setupToolButtons();
    setupSlots();
    setupFileExplorer();
}

LayerExplorerWidget::~LayerExplorerWidget() {
    delete ui;
}

void LayerExplorerWidget::setupToolButtons() {
    // ---------------------------------------------------------
    // @SECTION: сортировка слоев
    // ---------------------------------------------------------

    QMenu* sortingMenu = new QMenu(this);

    QAction* sortByAlphabetically = new QAction(tr("По алфавиту"));
    QAction* sortByTimestemp      = new QAction(tr("По времени"));
    QAction* sortOrderInverted    = sortingMenu->addAction(tr("В обратном порядке"));
    QAction* sortByNone           = new QAction(tr("Без сортировки"));

    sortByAlphabetically->setCheckable(true);
    sortByAlphabetically->setChecked(false);

    sortByTimestemp->setCheckable(true);
    sortByTimestemp->setChecked(false);

    sortOrderInverted->setCheckable(true);
    sortOrderInverted->setChecked(false);

    sortByNone->setChecked(true);
    sortByNone->setCheckable(true);

    QActionGroup* criteriaGroup = new QActionGroup(this);
    criteriaGroup->addAction(sortByAlphabetically);
    criteriaGroup->addAction(sortByTimestemp);
    criteriaGroup->setExclusive(true);

    sortingMenu->addAction(sortByAlphabetically);
    sortingMenu->addAction(sortByTimestemp);
    sortingMenu->addAction(sortByNone);
    sortingMenu->addSeparator();
    sortingMenu->addAction(sortOrderInverted);

    ui->toolButton_SortingLayers->setMenu(sortingMenu);

    connect(sortByAlphabetically, &QAction::triggered, this, [this, sortOrderInverted]() {
        on_sortByAlphabetically(!sortOrderInverted->isChecked());
    });
    connect(sortByTimestemp, &QAction::triggered, this, [this, sortOrderInverted]() {
        on_sortByTimestemp(!sortOrderInverted->isChecked());
    });
    connect(sortByNone, &QAction::triggered, this, [this, sortOrderInverted]() {
        sortOrderInverted->setEnabled(false); // Выключаем "Обратный порядок"
        // Вызываем слот сброса сортировки (восстановление исходного дерева)
        on_resetSortToDefault();
    });
    connect(sortOrderInverted, &QAction::triggered, this, [this, sortByAlphabetically, sortOrderInverted]() {
        if (sortByAlphabetically->isChecked()) {
            on_sortByAlphabetically(!sortOrderInverted->isChecked());
        } else {
            on_sortByTimestemp(!sortOrderInverted->isChecked());
        }
    });

    // ---------------------------------------------------------
    // @SECTION: Добавление элемента
    // ---------------------------------------------------------
    QMenu* addElementMenu = new QMenu(this);

    QAction* actionAddExperiment = new QAction(tr("Добавить эксперимент"));
    QAction* actionAddLayerGroup = new QAction(tr("Добавить снимок"));
    QAction* actionAddLayer      = new QAction(tr("Добавить слой (представление)"));

    addElementMenu->addAction(actionAddExperiment);
    addElementMenu->addAction(actionAddLayerGroup);
    addElementMenu->addAction(actionAddLayer);

    ui->toolButton_AddElement->setMenu(addElementMenu);
    connect(actionAddExperiment,
            &QAction::triggered,
            this,
            &LayerExplorerWidget::on_actionAddExperiment_clicked);
    connect(actionAddLayerGroup,
            &QAction::triggered,
            this,
            &LayerExplorerWidget::on_actionAddSnapshot_clicked);
    connect(actionAddLayer, &QAction::triggered, this, &LayerExplorerWidget::on_actionAddLayer_clicked);
}

void LayerExplorerWidget::setupSlots() {
    // ------ отвечает за отображение меню с настройками ------
    connect(ui->toolButton_VisiblePropertyInspector,
            &QToolButton::toggled,
            this,
            &LayerExplorerWidget::propertyInspectorVisibleRequested);

    connect(ui->lineEdit_root_path,
            &QLineEdit::textChanged,
            this,
            &LayerExplorerWidget::on_QLineEdit_rootPath_changed);

    connect(ui->treeView_Layers,
            &QTreeView::customContextMenuRequested,
            this,
            &LayerExplorerWidget::showContextMenu);
    connect(ui->treeView_Layers->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &LayerExplorerWidget::onNodeSelected);
    if (m_app && m_app->dataTreeModel()) {
        connect(m_app->dataTreeModel(),
                &QAbstractItemModel::dataChanged,
                this,
                [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles) {
                    // Проверяем, что изменилось именно состояние чекбокса
                    if (roles.contains(Qt::CheckStateRole)) {
                        QUuid id      = topLeft.data(TreeDataRole::IdRole).toUuid();
                        bool  checked = (topLeft.data(Qt::CheckStateRole).toInt() == Qt::Checked);

                        // Вызываем ваш сигнал обновления настроек в ObjectRegistry
                        emit updateNodeSettingsRequested(id, [checked](Core::VisualSettings& settings) {
                            settings.isVisible = checked;
                        });
                    }
                });
    }
}
void LayerExplorerWidget::sortTreeHierarchy(
    QTreeView*                                                  tree,
    std::function<bool(const QModelIndex&, const QModelIndex&)> comparator) {
    if (!tree)
        return;

    // Достаем прокси-модель
    auto* proxy = qobject_cast<CustomSortProxyModel*>(tree->model());
    if (!proxy)
        return;

    tree->setUpdatesEnabled(false);

    proxy->setComparator(comparator);

    // Сортируем по первой колонке (0) в возрастающем порядке (логика направления внутри компаратора)
    proxy->sort(0, Qt::AscendingOrder);

    tree->setUpdatesEnabled(true);
}

void LayerExplorerWidget::on_comboBox_fileStructure_changed(int index) {
    if (index < 0)
        return;
    m_currentVersion =
        static_cast<Core::ModelingProgrammVersion>(ui->comboBox_fileStructure->itemData(index).toInt());
}

void LayerExplorerWidget::on_comboBox_structureView_changed(int index) {
    if (!m_app || !m_app->dataTreeModel() || !ui->treeView_Layers) {
        return;
    }

    auto* treeModel = m_app->dataTreeModel();

    ui->treeView_Layers->blockSignals(true);

    // Исправляем пространство имен (сверьтесь со своим объявлением enum)
    auto mode = static_cast<Models::TreeMode>(index);

    treeModel->setTreeMode(mode);

    ui->treeView_Layers->blockSignals(false);

    if (mode == Models::TreeMode::ComponentView) {
        ui->treeView_Layers->expandAll();

    } else {
        // Запрашиваем модель, которая реально назначена во View (может быть прокси)
        auto* currentViewModel = ui->treeView_Layers->model();
        if (currentViewModel) {
            for (int i = 0; i < currentViewModel->rowCount(); ++i) {
                // Получаем индекс, валидный для текущего View
                QModelIndex expIndex = currentViewModel->index(i, 0);
                ui->treeView_Layers->setExpanded(expIndex, true);
            }
        }
    }
}

void LayerExplorerWidget::on_QLineEdit_findLayer_changed(const QString& line) {
    // Достаем прокси-модель из вью
    auto* proxy = qobject_cast<QSortFilterProxyModel*>(ui->treeView_Layers->model());
    if (proxy) {
        // Передаем регулярное выражение или фиксированную строку для фильтра
        proxy->setFilterFixedString(line);
    }
}

void LayerExplorerWidget::setupFileExplorer() {
    // 1. Создаем готовую модель файловой системы
    QFileSystemModel* fileModel = new QFileSystemModel(this);

    // Указываем корневой путь (модель начнет асинхронно сканировать диск отсюда)
    // Для теста можно жестко зашить, а в будущем брать из настроек проекта
    m_root_path = QCoreApplication::applicationDirPath();
    // На всякий случай проверяем (хотя папка запуска обязана существовать)
    if (!QDir(m_root_path).exists()) {
        m_root_path = QDir::currentPath(); // Альтернативный вариант (рабочая директория)
    }
    fileModel->setRootPath(m_root_path);

    // 2. Настраиваем фильтр файлов (отображаем только нужные форматы)
    fileModel->setNameFilters(QStringList() << "*.bin" << "*.hdf5" << "*.csv" << "*.dat");
    // Если false -> файлы, не прошедшие фильтр, будут скрыты (а не просто задизейблены)
    fileModel->setNameFilterDisables(false);

    // По умолчанию модель показывает и папки. Если нужно скрыть скрытые/системные файлы:
    fileModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    // 3. Связываем модель с отображением (QTreeView)
    ui->treeViewFiles->setModel(fileModel);

    // Важно: говорим TreeView отображать дерево именно с нашей корневой папки,
    // иначе он покажет весь компьютер (Мой компьютер, Диск C, Диск D и т.д.)
    ui->treeViewFiles->setRootIndex(fileModel->index(m_root_path));

    // 4. Тонкая настройка внешнего вида (Кастомизация под ваш интерфейс)
    // Скрываем ненужные колонки, если вам нужно только имя файла:
    ui->treeViewFiles->setColumnHidden(1, true); // Скрыть колонку "Размер"
    ui->treeViewFiles->setColumnHidden(2, true); // Скрыть колонку "Тип"
    ui->treeViewFiles->setColumnHidden(3, true); // Скрыть колонку "Дата изменения"

    // Растягиваем оставшуюся колонку с именем на всю ширину панели
    ui->treeViewFiles->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    // Разрешаем раскрывать папки по клику
    ui->treeViewFiles->setAnimated(true);
    ui->treeViewFiles->setSortingEnabled(true); // Разрешаем сортировку по алфавиту
}

// Сортировка внутри экспериментов по алфавиту названий снапшотов
void LayerExplorerWidget::on_sortByAlphabetically(const bool direct) {
    sortTreeHierarchy(ui->treeView_Layers, [direct](const QModelIndex& a, const QModelIndex& b) {
        int result = QString::compare(a.data(Qt::DisplayRole).toString(),
                                      b.data(Qt::DisplayRole).toString(),
                                      Qt::CaseInsensitive);
        return direct ? (result < 0) : (result > 0);
    });
}

// Сортировка внутри экспериментов по физическому моменту времени (Timestamp) снапшота
void LayerExplorerWidget::on_sortByTimestemp(const bool direct) {
    sortTreeHierarchy(ui->treeView_Layers, [direct](const QModelIndex& a, const QModelIndex& b) {
        // Вытаскиваем Timestamp напрямую из индексов исходной модели
        qint64 timeA = a.data(TreeDataRole::TimestampRole).toLongLong();
        qint64 timeB = b.data(TreeDataRole::TimestampRole).toLongLong();

        return direct ? (timeA < timeB) : (timeA > timeB);
    });
}
// Сброс к хронологическому порядку добавления снапшотов в UI
void LayerExplorerWidget::on_resetSortToDefault() {
    // Вызываем нашу новую функцию сортировки, передавая treeView_Layers и компаратор
    sortTreeHierarchy(ui->treeView_Layers, [](const QModelIndex& a, const QModelIndex& b) {
        // Извлекаем порядковый индекс, сохраненный при добавлении ноды в модель
        int indexA = a.data(TreeDataRole::DefaultOrderRole).toInt();
        int indexB = b.data(TreeDataRole::DefaultOrderRole).toInt();

        // Возвращаем true, если первый элемент был добавлен раньше второго.
        // Это восстановит исходный "прямой" порядок отображения.
        return indexA < indexB;
    });
}

void LayerExplorerWidget::on_actionAddLayer_clicked() {
    // 1. Проверяем инициализацию ядра и представления дерева слоев
    if (!m_app || !ui || !ui->treeView_Layers) {
        return;
    }

    // 2. Получаем текущий выделенный индекс из представления (это индекс прокси-модели)
    QModelIndex proxyIndex = ui->treeView_Layers->currentIndex();
    if (!proxyIndex.isValid()) {
        // Если ничего не выбрано, выходим
        return;
    }

    // 3. Преобразуем прокси-индекс в индекс оригинальной DataTreeModel
    auto*       proxy       = qobject_cast<QSortFilterProxyModel*>(ui->treeView_Layers->model());
    QModelIndex sourceIndex = proxy ? proxy->mapToSource(proxyIndex) : proxyIndex;

    // 4. Извлекаем UUID ноды данных, используя вашу кастомную роль
    QUuid nodeId = sourceIndex.data(TreeDataRole::IdRole).toUuid();
    if (nodeId.isNull()) {
        // Кликнули по элементу, у которого нет UUID (например, по пустой группе или корню)
        return;
    }

    // 5. Делегируем бизнес-логику в DataController ядра системы
    const auto dataController = m_app->dataController();
    if (dataController) {
        dataController->createLayerForNode(nodeId);
    }
}

void LayerExplorerWidget::on_actionAddSnapshot_clicked() {
    QMessageBox::information(this, tr("Внимание"), tr("Функция добавления снапшота пока не реализована"));
    // if (ui->comboBox_structureView->currentIndex() == 0)
    //     return;
    // // 1. UI-логика: Запрашиваем у пользователя файл(ы) снапшота
    // // Используем m_root_path как стартовую директорию для удобства
    // QStringList filePaths =
    //     QFileDialog::getOpenFileNames(this,
    //                                   tr("Выберите файлы снапшота (таймстепа)"),
    //                                   m_root_path,
    //                                   tr("Файлы симуляции (*.bin *.hdf5 *.csv *.dat);;Все файлы (*.*)"));

    // // Если пользователь отменил выбор, просто выходим
    // if (filePaths.isEmpty()) {
    //     return;
    // }

    // // Обновляем m_root_path путем последнего выбранного файла, чтобы при следующем открытии
    // // диалог распахивался в этой же папке
    // m_root_path = QFileInfo(filePaths.first()).absolutePath();

    // // 2. Извлекаем базовое имя файла для названия узла в дереве
    // // Например, если файл "snapshot_001.bin", имя будет "snapshot_001"
    // QString snapshotName = QFileInfo(filePaths.first()).baseName();

    // // 3. Бизнес-логика: Передаем задачу импорта в Ядро системы
    // if (!m_app) {
    //     return;
    // }

    // const auto dataController = m_app->dataController();
    // if (dataController) {
    //     // Используем метод импорта эксперимента.
    //     // Если у вас один файл — ядро создаст для него узел (DataNode) внутри структуры,
    //     // используя snapshotName как метку, и применит выбранную в комбобоксе версию формата.
    //     dataController->importExperiment(snapshotName, filePaths, m_currentVersion);
    // }
}

void LayerExplorerWidget::on_actionAddExperiment_clicked() {
    // Запрашиваем директорию эксперимента
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Выберите папку эксперимента"), m_root_path);
    if (dirPath.isEmpty())
        return;

    if (m_app && m_app->dataController()) {
        m_app->dataController()->importExperiment(dirPath, m_currentVersion);
    }
}

void LayerExplorerWidget::on_pushButton_removeElement_clicked() {
    QList<QUuid> ids = getSelectedIds();
    for (const QUuid& id : ids) {
        // Делегируем удаление через сигналы, AppCore удалит ноду из ObjectRegistry
        emit removalRequested(id);
    }
}
void LayerExplorerWidget::on_pushButton_changeRootPath_clicked() {
    QString path = QFileDialog::getExistingDirectory();
    if (!QFileInfo(path).exists() || path.isEmpty()) {
        return;
    }
    m_root_path = path;
};

void LayerExplorerWidget::onNodeSelected() {
    QList<QUuid> selectedIds = getSelectedIds();
    // Оповещаем мир о массовом изменении
    emit selectionChanged(selectedIds);
}
QList<QUuid> LayerExplorerWidget::getSelectedIds() const {
    QList<QUuid> ids;
    // Забираем выделенные индексы первой колонки
    auto indexes = ui->treeView_Layers->selectionModel()->selectedRows(0);

    for (const QModelIndex& index : indexes) {
        QUuid id = index.data(TreeDataRole::IdRole).toUuid();
        if (!id.isNull()) {
            ids.append(id);
        }
    }
    return ids;
}

void LayerExplorerWidget::onNodeAdded(std::shared_ptr<QSpace::Core::DataNode> node) {
    // ui->treeView_Layers->blockSignals(true);
    // auto* item = new QtreeView_LayersItem(ui->treeView_Layers);
    // item->setText(0, node->label);
    // item->setData(0, TreeDataRole::IdRole, node->id);
    // item->setData(0, TreeDataRole::TimestampRole, node->stats.timestamp);
    // item->setData(0, TreeDataRole::DefaultOrderRole, ui->treeView_Layers->topLevelItemCount());

    // item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    // item->setCheckState(0, node->masterSettings->isVisible ? Qt::Checked : Qt::Unchecked);
    // ui->treeView_Layers->blockSignals(false);
}
void LayerExplorerWidget::onObjectRemoved(const QUuid& id) {
    // QtreeView_LayersItem* item = findTreeElementById(id);
    // if (item) {
    //     delete item;
    // }
}
void LayerExplorerWidget::showContextMenu(const QPoint& pos) {
    // находим элемент по позиции
    QtreeView_LayersItem* item = ui->treeView_Layers->itemAt(pos);
    if (!item)
        return;
    QUuid id = QUuid::fromString(item->data(0, Qt::UserRole).toString());
    // создаем меню в данном месте
    QMenu    menu;
    QAction* action = menu.addAction(tr("delete layer"));
    // QAction* action1 = menu.addAction((tr("open visual properties")));

    // если пользователь нажмет удалить то будет вызвано действие удаления объекта
    connect(action, &QAction::triggered, this, [this, &id]() { emit removalRequested(id); });

    menu.exec(ui->treeView_Layers->viewport()->mapToGlobal(pos));
}
void LayerExplorerWidget::onItemChanged(QtreeView_LayersItem* item, int col) {
    if (col != 0)
        return;
    QString idStr = item->data(0, Qt::UserRole).toString();
    if (idStr.isEmpty())
        return;
    QUuid id      = QUuid::fromString(idStr);
    bool  checked = (item->checkState(0) == Qt::Checked);
    emit  updateNodeSettingsRequested(id, [checked](Core::VisualSettings& settings) {
        settings.isVisible = checked;
    });
}
QtreeView_LayersItem* LayerExplorerWidget::findTreeElementById(const QUuid& id) {
    QString idStr = id.toString();
    // Проходим по всем элементам верхнего уровня
    for (int i = 0; i < ui->treeView_Layers->topLevelItemCount(); ++i) {
        QtreeView_LayersItem* topItem = ui->treeView_Layers->topLevelItem(i);
        if (topItem->data(0, Qt::UserRole).toString() == idStr) {
            return topItem;
        }

        // Ищем в детях (если это контейнер/группа)
        for (int j = 0; j < topItem->childCount(); ++j) {
            QtreeView_LayersItem* child = topItem->child(j);
            if (child->data(0, Qt::UserRole).toString() == idStr) {
                return child;
            }
        }
    }
    return nullptr;
}
// ---------------------------------------------------------
// @SECTION: Редактор слоев - Видимость слоев
// ---------------------------------------------------------
void LayerExplorerWidget::on_pushButton_hideAll_clicked() {
    QtreeView_LayersItemIterator it(ui->treeView_Layers);
    while (*it) {
        if ((*it)->flags() & Qt::ItemIsUserCheckable) {
            (*it)->setCheckState(0, Qt::Unchecked);
        }
        ++it;
    }
}
void LayerExplorerWidget::on_pushButton_showAll_clicked() {
    QtreeView_LayersItemIterator it(ui->treeView_Layers);
    while (*it) {
        if ((*it)->flags() & Qt::ItemIsUserCheckable) {
            (*it)->setCheckState(0, Qt::Checked);
        }
        ++it;
    }
}
void LayerExplorerWidget::on_pushButton_hideSelected_clicked() {
    for (auto* item : ui->treeView_Layers->selectedItems()) {
        if (item->flags() & Qt::ItemIsUserCheckable) {
            item->setCheckState(0, Qt::Unchecked);
        }
    }
}
void LayerExplorerWidget::on_pushButton_showSelected_clicked() {
    for (auto* item : ui->treeView_Layers->selectedItems()) {
        if (item->flags() & Qt::ItemIsUserCheckable) {
            item->setCheckState(0, Qt::Checked);
        }
    }
}
void LayerExplorerWidget::on_pushButton_hideUnselected_clicked() {
    QtreeView_LayersItemIterator it(ui->treeView_Layers);
    while (*it) {
        if (!(*it)->isSelected() && ((*it)->flags() & Qt::ItemIsUserCheckable)) {
            (*it)->setCheckState(0, Qt::Unchecked);
        }
        ++it;
    }
}
// ---------------------------------------------------------
// @SECTION: Редактор слоев - Фильтры слоев
// ---------------------------------------------------------
void LayerExplorerWidget::on_checkBox_showLoadedLayers_changed(const bool isChecked) {
    // Заглушка: Здесь можно реализовать проверку статуса ноды (загружена ли она в LRU кэш)
    // и скрывать/отображать соответствующие элементы QtreeView_Layers.
}
void LayerExplorerWidget::on_checkBox_showUnLoadedLayers_changed(const bool isChecked) {
    // Заглушка: Аналогично для выгруженных слоев
}
void LayerExplorerWidget::on_checkBox_FilterEquation_changed(const bool isChecked) {
    // Активация фильтрации слоев на основе пользовательского уравнения/выражения
}
void LayerExplorerWidget::on_pushButton_changeFilterEquationLine_clicked(const QString& line) {
    // Обработка изменения выражения для фильтрации
}
// ---------------------------------------------------------
// @SECTION: Управление структурой
// ---------------------------------------------------------
void LayerExplorerWidget::on_pushButton_expandAll_clicked() {
    ui->treeView_Layers->expandAll();
}
void LayerExplorerWidget::on_pushButton_collapseAll_clicked() {
    ui->treeView_Layers->collapseAll();
}
void LayerExplorerWidget::on_pushButton_CollapseAllFiles_clicked() {
    ui->treeViewFiles->collapseAll();
}
// ---------------------------------------------------------
// @SECTION: Проводник файлов
// ---------------------------------------------------------
void LayerExplorerWidget::on_QLineEdit_rootPath_changed(const QString& line) {
    QDir dir(line);
    if (dir.exists()) {
        m_root_path = line;
        if (auto* model = qobject_cast<QFileSystemModel*>(ui->treeViewFiles->model())) {
            ui->treeViewFiles->setRootIndex(model->index(m_root_path));
        }
    }
}
void LayerExplorerWidget::on_QLineEdit_findFile_changed(const QString& line) {
    if (auto* model = qobject_cast<QFileSystemModel*>(ui->treeViewFiles->model())) {
        QStringList filters;
        if (line.isEmpty()) {
            // Возвращаем исходные фильтры
            filters << "*.bin" << "*.hdf5" << "*.csv" << "*.dat";
        } else {
            // Добавляем wildcard для поиска по подстроке
            filters << QString("*%1*").arg(line);
        }
        model->setNameFilters(filters);
    }
}
void LayerExplorerWidget::on_pushButton_importData_clicked() {
    QModelIndex index = ui->treeViewFiles->currentIndex();
    if (!index.isValid())
        return;

    auto* model = qobject_cast<QFileSystemModel*>(ui->treeViewFiles->model());
    if (!model)
        return;

    QString filePath = model->filePath(index);
    if (QFileInfo(filePath).isFile()) {
        // Ядро отвечает за создание уникальной ноды. UI просто передает путь.
        const auto dataController = m_app->dataController();
        if (dataController)
            dataController->importExperiment(QFileInfo(filePath).baseName(), {filePath});
    }
}
void LayerExplorerWidget::on_pushButton_removeData_clicked() {
    // Метод предназначен для выгрузки данных из ОЗУ.
    // Если нужно выгрузить файл из LRU Cache, собираем UUID выделенных элементов и отдаем команду в ядро:
    QList<QUuid> ids = getSelectedIds();
    for (const auto& id : ids) {
        // m_app->unloadDataFromCache(id); // Пример вызова, зависит от реализации AppCore
    }
}
// ---------------------------------------------------------
// @SECTION: Дополнительные методы
// ---------------------------------------------------------

void LayerExplorerWidget::addComponent() {
    // Делегируем в ядро создание компонента для текущего снапшота
    QModelIndex currentIndex = ui->treeView_Layers->currentIndex();
    if (!currentIndex.isValid())
        return;

    QUuid nodeId = currentIndex.data(UI::TreeDataRole::IdRole).toUuid();
    if (!nodeId.isNull()) {
        // m_app->createComponent(nodeId);
    }
}

void LayerExplorerWidget::addSnapshot() {
    // Вызов добавления снапшота. Можно использовать QFileDialog, как в addExperiment
    on_actionAddExperiment_clicked();
}
} // namespace QSpace::UI