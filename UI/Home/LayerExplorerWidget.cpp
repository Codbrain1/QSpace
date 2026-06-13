#include "LayerExplorerWidget.h"
#include "Common/Logger/Logger.h"
#include "Core/AppCore/AppCore.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Enums/CoreEnums.h"
#include "Models/DataTreeModel/DataTreeModel.h"
#include "SelectExperimentDialog.h"
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
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qloggingcategory.h>
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
    Q_OBJECT
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
        auto* actualDataModel = m_app->dataTreeModel(); // Получаем прямой указатель

        auto* proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(actualDataModel);
        proxyModel->setFilterKeyColumn(0); // Фильтруем по первой колонке (название слоя)
        proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

        // В QTreeView устанавливаем ИМЕННО прокси-модель
        ui->treeView_Layers->setModel(proxyModel);

        // Сохраняем указатель в поле класса (если m_treeModel объявлен в хедере),
        m_treeModel = actualDataModel;

        // даем команду на первичное построение дерева. Сигналы гарантированно дойдут до UI.
        m_treeModel->rebuildTree();
        handleExpandAll();
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
    auto* layout = ui->horizontalLayout_3;
    for (int i = 0; i < layout->count(); ++i) {
        auto* toolbutton = qobject_cast<QToolButton*>(layout->itemAt(i)->widget());
        if (toolbutton) {
            toolbutton->setFixedSize(30, 30);
        }
    }
    // ---------------------------------------------------------
    // @SECTION: сортировка слоев
    // ---------------------------------------------------------

    QMenu* sortingMenu = new QMenu(this);

    QAction* sortByAlphabetically = new QAction(tr("Сортирвать по алфавиту"));
    QAction* sortByTimestemp      = new QAction(tr("Сортирвать по времени"));
    QAction* sortOrderInverted    = new QAction(tr("Сортирвать в обратном порядке"));
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
    criteriaGroup->addAction(sortByNone);
    criteriaGroup->setExclusive(true);

    sortingMenu->addAction(sortByAlphabetically);
    sortingMenu->addAction(sortByTimestemp);
    sortingMenu->addAction(sortByNone);
    sortingMenu->addSeparator();
    sortingMenu->addAction(sortOrderInverted);

    ui->toolButton_SortingLayers->setMenu(sortingMenu);

    connect(sortByAlphabetically, &QAction::triggered, this, [this, sortOrderInverted]() {
        handleSortByAlphabetically(!sortOrderInverted->isChecked());
    });
    connect(sortByTimestemp, &QAction::triggered, this, [this, sortOrderInverted]() {
        handleSortByTimestemp(!sortOrderInverted->isChecked());
    });
    connect(sortByNone, &QAction::triggered, this, [this, sortOrderInverted]() {
        sortOrderInverted->setEnabled(false); // Выключаем "Обратный порядок"
        // Вызываем слот сброса сортировки (восстановление исходного дерева)
        handleResetSortToDefault();
    });
    connect(sortOrderInverted, &QAction::triggered, this, [this, sortByAlphabetically, sortOrderInverted]() {
        if (sortByAlphabetically->isChecked()) {
            handleSortByAlphabetically(!sortOrderInverted->isChecked());
        } else {
            handleSortByTimestemp(!sortOrderInverted->isChecked());
        }
    });

    // ---------------------------------------------------------
    // @SECTION: Добавление элемента
    // ---------------------------------------------------------
    QMenu* addElementMenu = new QMenu(this);

    QAction* actionAddLayer      = new QAction(tr("Добавить слой (представление)"));
    QAction* actionImportFiles   = new QAction(tr("Импортировать файлы с данными"));
    QAction* actionAddExperiment = new QAction(tr("Импортировать эксперимент"));

    addElementMenu->addAction(actionAddLayer);
    addElementMenu->addAction(actionImportFiles);
    addElementMenu->addAction(actionAddExperiment);

    ui->toolButton_AddElement->setMenu(addElementMenu);
    connect(actionAddExperiment, &QAction::triggered, this, &LayerExplorerWidget::handleAddExperiment);
    connect(actionImportFiles,
            &QAction::triggered,
            this,
            &LayerExplorerWidget::handleImportFilesRequestFromLayerEditor);
    connect(actionAddLayer, &QAction::triggered, this, &LayerExplorerWidget::handleAddLayer);
}

void LayerExplorerWidget::setupSlots() {
    // --------- Менеджер Файлов ---------
    // изменение корневой директории для проводника файлов
    connect(ui->lineEdit_root_path,
            &QLineEdit::textChanged,
            this,
            &LayerExplorerWidget::handleRootPathChange);
    // выбор корневой директории для проводника файлов
    connect(ui->pushButton_change_root, &QPushButton::clicked, this, [this]() {
        const QString dirPath =
            QFileDialog::getExistingDirectory(this, tr("Пожалуйста выберите директорию"), m_root_path);
        handleRootPathChange(dirPath);
    });
    //

    // --------- Менеджер слоев ---------
    // сообщает ядру об активации ноды
    connect(this,
            &LayerExplorerWidget::nodeSelectionActivated,
            m_app->dataController(),
            &QSpace::Core::Controllers::DataController::onNodeSelectionActivated,
            Qt::QueuedConnection);
    // отвечает за отображение меню с настройками
    connect(ui->toolButton_VisiblePropertyInspector,
            &QToolButton::toggled,
            this,
            &LayerExplorerWidget::propertyInspectorVisibleRequested);
    // изменение структуры отображения для проводника данных (слоев)
    connect(ui->comboBox_structureView,
            &QComboBox::currentIndexChanged,
            this,
            &LayerExplorerWidget::handleStructureViewChange);
    // изменение схемы чтения данных
    connect(ui->comboBox_fileStructure,
            &QComboBox::currentIndexChanged,
            this,
            &LayerExplorerWidget::handleReadSchenmeChange);
    // поиск в проводнике слоев
    connect(ui->lineEdit_findLayer,
            &QLineEdit::textChanged,
            this,
            &LayerExplorerWidget::handleFindLayerChange);
    // вызов отрисовки меню по нажатию ПКМ
    connect(ui->treeView_Layers,
            &QTreeView::customContextMenuRequested,
            this,
            &LayerExplorerWidget::handleShowCustomContexMenuForTreeViewElement);

    // отправляет сигнал в MainWindow о том какая сейчас выбрана запись, что позволяет на лету менять вид для
    // меню настроек
    connect(ui->treeView_Layers->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &LayerExplorerWidget::handleNodeSelected);

    // удаление выбранных элементов
    connect(ui->toolButton_RemoveElement,
            &QToolButton::toggled,
            this,
            &LayerExplorerWidget::handleRemoveElement);

    if (m_app && m_app->dataTreeModel()) {
        connect(m_app->dataTreeModel(),
                &QAbstractItemModel::dataChanged,
                this,
                [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int>& roles) {
                    // Проверяем, что изменилось именно состояние чекбокса
                    if (roles.contains(Qt::CheckStateRole)) {
                        QUuid id      = topLeft.data(Models::DataTreeModel::CustomRoles::IdRole).toUuid();
                        bool  checked = (topLeft.data(Qt::CheckStateRole).toInt() == Qt::Checked);

                        // Вызываем ваш сигнал обновления настроек в ObjectRegistry
                        emit updateNodeSettingsRequested(id, [checked](Core::VisualSettings& settings) {
                            settings.isVisible = checked;
                        });
                    }
                });
        connect(this,
                &LayerExplorerWidget::removalRequested,
                m_app->dataController(),
                &Core::Controllers::DataController::removeNodeObject,
                Qt::QueuedConnection);
    }
}
void LayerExplorerWidget::sortTreeHierarchyInternal(
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

void LayerExplorerWidget::handleReadSchenmeChange(int index) {
    if (index < 0)
        return;
    m_currentVersion =
        static_cast<Core::ModelingProgrammVersion>(ui->comboBox_fileStructure->itemData(index).toInt());
}
void LayerExplorerWidget::handleShowCustomContexMenuForTreeViewElement(const QPoint& pos) {
    if (!ui || !ui->treeView_Layers) {
        return;
    }
    // 1. Получаем индекс элемента (прокси-индекс) под курсором мыши
    QModelIndex proxyIndex = ui->treeView_Layers->indexAt(pos);

    // Если кликнули в пустую область (не по элементу), можно либо выйти,
    // либо показать какое-то базовое меню (например, "Добавить эксперимент")
    if (!proxyIndex.isValid()) {
        qCWarning(LogUI) << "proxy index is invalid for position: " << pos;
        return;
    }
    auto*       proxy       = qobject_cast<QSortFilterProxyModel*>(ui->treeView_Layers->model());
    QModelIndex sourceIndex = proxy ? proxy->mapToSource(proxyIndex) : proxyIndex;
    auto*       item        = static_cast<Models::DataTreeItem*>(sourceIndex.internalPointer());
    if (!item) {
        return;
    }
    // 4. Создаем контекстное меню
    QMenu contextMenu(this);
    // 5. Вызываем соответствующий метод-помощник в зависимости от типа узла
    // Передаем в методы само меню (чтобы наполнить его action'ами) и sourceIndex (чтобы знать, для кого меню)
    switch (item->type()) {
        case Models::DataTreeItem::Experiment:
            showCustomContextMenuForExperimentInternal(&contextMenu, sourceIndex);
            break;
        case Models::DataTreeItem::Snapshot:
            showCustomContextMenuForSnapshotInternal(&contextMenu, sourceIndex);
            break;
        case Models::DataTreeItem::ComponentGroup:
            showCustomContextMenuForComponentGroupInternal(&contextMenu, sourceIndex);
            break;
        case Models::DataTreeItem::DataNode:
            showCustomContextMenuForDataNodeInternal(&contextMenu, sourceIndex);
            break;
        case Models::DataTreeItem::LayerItem:
            showCustomContextMenuForLayerInternal(&contextMenu, sourceIndex);
            break;
        default:
            break; // Для Root или неизвестных типов меню не показываем
    }
    if (!contextMenu.isEmpty()) {
        // Обязательно конвертируем локальные координаты QTreeView в глобальные координаты экрана
        contextMenu.exec(ui->treeView_Layers->viewport()->mapToGlobal(pos));
    }
}

void LayerExplorerWidget::showCustomContextMenuForExperimentInternal(QMenu* menu, const QModelIndex& index) {
}
void LayerExplorerWidget::showCustomContextMenuForSnapshotInternal(QMenu* menu, const QModelIndex& index) {
}
void LayerExplorerWidget::showCustomContextMenuForDataNodeInternal(QMenu* menu, const QModelIndex& index) {
    QUuid    dataNodeId     = index.data(Models::DataTreeModel::CustomRoles::IdRole).toUuid();
    QAction* addLayerAction = menu->addAction(tr("Добавить слой"));
    QAction* loadDataAction = menu->addAction(tr("Загрузить данные"));
    connect(loadDataAction, &QAction::triggered, this, [this, dataNodeId]() {
        emit nodeSelectionActivated(dataNodeId);
    });
    connect(addLayerAction, &QAction::triggered, this, [this, dataNodeId]() {
        const auto dataController = m_app->dataController();
        if (dataController) {
            dataController->createLayerForNode(dataNodeId);
        }
    });
}
void LayerExplorerWidget::showCustomContextMenuForComponentGroupInternal(QMenu*             menu,
                                                                         const QModelIndex& index) {
}
void LayerExplorerWidget::showCustomContextMenuForLayerInternal(QMenu* menu, const QModelIndex& index) {
    // Получаем UUID конкретного слоя
    QUuid layerId = index.data(Models::DataTreeModel::CustomRoles::IdRole).toUuid();

    // Создаем Action
    QAction* deleteAction = menu->addAction(tr("Удалить слой"));
    // deleteAction->setIcon(QIcon(":/icons/delete.png")); // Если есть иконка

    // Подключаем логику удаления
    connect(deleteAction, &QAction::triggered, this, [this, layerId]() {
        // Здесь обращаемся к вашему LayerManager для удаления
        emit removalRequested(layerId);
    });
}

void LayerExplorerWidget::handleStructureViewChange(int index) {
    if (!m_app || !m_app->dataTreeModel() || !ui->treeView_Layers) {
        return;
    }

    auto* treeModel = m_app->dataTreeModel();
    auto  mode      = static_cast<Models::DataTreeModel::TreeMode>(index);

    // 1. Переключаем режим внутри модели.
    // Модель сама вызовет beginResetModel/endResetModel, View обновится автоматически.
    treeModel->setTreeMode(mode);

    // 2. Управляем раскрытием дерева (Умный UX)
    // Запрашиваем актуальную модель у View (на случай, если используется QSortFilterProxyModel)
    auto* currentViewModel = ui->treeView_Layers->model();
    if (!currentViewModel) {
        return;
    }

    // Блокируем отрисовку на время массового изменения состояния веток
    ui->treeView_Layers->updatesEnabled(); // Альтернатива blockSignals для UI

    // Сначала сворачиваем всё, чтобы убрать артефакты от предыдущего режима
    ui->treeView_Layers->collapseAll();

    // Обходим дерево на нужную нам глубину (до 2-го уровня включительно)
    for (int i = 0; i < currentViewModel->rowCount(); ++i) {
        // Уровень 1: Эксперименты (раскрываем всегда)
        QModelIndex expIndex = currentViewModel->index(i, 0);
        ui->treeView_Layers->setExpanded(expIndex, true);

        // Уровень 2: Снапшоты или Группы компонент
        int childCount = currentViewModel->rowCount(expIndex);
        for (int j = 0; j < childCount; ++j) {
            QModelIndex childIndex = currentViewModel->index(j, 0, expIndex);

            // Раскрываем Снапшот во временном виде ИЛИ Группу (Газ/Звезды) в плоском виде
            ui->treeView_Layers->setExpanded(childIndex, true);

            // Ноды (файлы данных) и Слои внутри них остаются свернутыми!
        }
    }

    ui->treeView_Layers->setUpdatesEnabled(true);
}

void LayerExplorerWidget::handleFindLayerChange(const QString& line) {
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
    ui->treeViewFiles->setSortingEnabled(true);                     // Разрешаем сортировку по алфавиту
    ui->treeViewFiles->setContextMenuPolicy(Qt::CustomContextMenu); // Для будущего контекстного меню

    connect(ui->treeViewFiles,
            &QTreeView::customContextMenuRequested,
            this,
            &LayerExplorerWidget::handleShowCustomContextMenuForFile);
}

// Сортировка внутри экспериментов по алфавиту названий снапшотов
void LayerExplorerWidget::handleSortByAlphabetically(const bool direct) {
    sortTreeHierarchyInternal(ui->treeView_Layers, [direct](const QModelIndex& a, const QModelIndex& b) {
        int result = QString::compare(a.data(Qt::DisplayRole).toString(),
                                      b.data(Qt::DisplayRole).toString(),
                                      Qt::CaseInsensitive);
        return direct ? (result < 0) : (result > 0);
    });
}

// Сортировка внутри экспериментов по физическому моменту времени (Timestamp) снапшота
void LayerExplorerWidget::handleSortByTimestemp(const bool direct) {
    sortTreeHierarchyInternal(ui->treeView_Layers, [direct](const QModelIndex& a, const QModelIndex& b) {
        // Вытаскиваем Timestamp напрямую из индексов исходной модели
        qint64 timeA = a.data(Models::DataTreeModel::CustomRoles::TimestampRole).toLongLong();
        qint64 timeB = b.data(Models::DataTreeModel::CustomRoles::TimestampRole).toLongLong();

        return direct ? (timeA < timeB) : (timeA > timeB);
    });
}
// Сброс к хронологическому порядку добавления снапшотов в UI
void LayerExplorerWidget::handleResetSortToDefault() {
    // Вызываем нашу новую функцию сортировки, передавая treeView_Layers и компаратор
    sortTreeHierarchyInternal(ui->treeView_Layers, [](const QModelIndex& a, const QModelIndex& b) {
        // Извлекаем порядковый индекс, сохраненный при добавлении ноды в модель
        int indexA = a.data(Models::DataTreeModel::CustomRoles::DefaultOrderRole).toInt();
        int indexB = b.data(Models::DataTreeModel::CustomRoles::DefaultOrderRole).toInt();

        // Возвращаем true, если первый элемент был добавлен раньше второго.
        // Это восстановит исходный "прямой" порядок отображения.
        return indexA < indexB;
    });
}

void LayerExplorerWidget::handleAddLayer() {
    // 1. Проверяем инициализацию ядра и представления дерева слоев
    if (!m_app || !ui || !ui->treeView_Layers) {
        return;
    }

    // 2. Получаем текущий выделенный индекс из представления (это индекс прокси-модели)
    QModelIndex proxyIndex = ui->treeView_Layers->currentIndex();
    if (!proxyIndex.isValid()) {
        QMessageBox::information(this,
                                 tr("Внимание"),
                                 tr("Чтобы создать представление данных выберите их в проводнике слоев"));
        // Если ничего не выбрано, выходим
        return;
    }

    // 3. Преобразуем прокси-индекс в индекс оригинальной DataTreeModel
    auto*       proxy       = qobject_cast<QSortFilterProxyModel*>(ui->treeView_Layers->model());
    QModelIndex sourceIndex = proxy ? proxy->mapToSource(proxyIndex) : proxyIndex;

    // 4. Извлекаем UUID ноды данных, используя вашу кастомную роль
    QUuid nodeId = sourceIndex.data(Models::DataTreeModel::CustomRoles::IdRole).toUuid();
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

void LayerExplorerWidget::handleImportFilesRequestFromLayerEditor() {
    auto result = selectExperimentDialogInternal();
    if (result.has_value()) {
        QStringList filePaths =
            QFileDialog::getOpenFileNames(this,
                                          tr("Выберите файлы с данными"),
                                          m_root_path,
                                          tr("Файлы симуляции (*.bin *.hdf5 *.csv);;Все файлы (*.*)"));
        if (filePaths.isEmpty())
            return;

        selectAndImportFilesInternal(result.value(), filePaths);
    }
}

void LayerExplorerWidget::handleImportFileRequestFromFileExplorer() {
    QModelIndex currentIndex = ui->treeViewFiles->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::information(this, tr("Внимание"), tr("Пожалуйста, выберите файл для импорта"));
        return;
    }

    QString filePath = currentIndex.data(QFileSystemModel::FilePathRole).toString();
    if (filePath.isEmpty()) {
        qCWarning(LogUI) << "Selected File path is empty.";
        return;
    }

    auto* dataController = m_app->dataController();
    if (!dataController) {
        qCCritical(LogUI) << "DataController pointer is segmentation fault!";
        return;
    }
    auto existingNodeId_opt = dataController->getNodeIdByFilePath(filePath);
    if (existingNodeId_opt.has_value()) {
        emit nodeSelectionActivated(existingNodeId_opt.value());
        qCInfo(LogIO) << "Data file added at existing node" << existingNodeId_opt.value();
        return;
    }
    auto result = selectExperimentDialogInternal();
    if (result.has_value()) {
        selectAndImportFilesInternal(result.value(), {filePath});
    }
}

std::optional<SelectExperimentDialogResult> LayerExplorerWidget::selectExperimentDialogInternal() {
    auto experiments =
        m_app->dataController()->getExperiments(); // Получаем список экспериментов из контроллера

    SelectExperimentDialog dialog(experiments, this);
    if (dialog.exec() != QDialog::Accepted) {
        return std::nullopt; // Пользователь отменил выбор
    }
    auto result = dialog.getResult();
    return result;
}

void LayerExplorerWidget::selectAndImportFilesInternal(const SelectExperimentDialogResult& result,
                                                       const QStringList&                  filePaths) {
    if (filePaths.isEmpty())
        return;

    if (result.isNewExperiment) {
        m_app->dataController()->importExperiment(filePaths, result.newExperimentName, m_currentVersion);
    } else {
        m_app->dataController()->importFiles(filePaths, m_currentVersion, result.exisitingExperimentId);
    }
}
// void LayerExplorerWidget::on_actionAddSnapshot_clicked() {
//     QMessageBox::information(this, tr("Внимание"), tr("Функция добавления снапшота пока не
//     реализована"));
//     // if (ui->comboBox_structureView->currentIndex() == 0)
//     //     return;
//     // // 1. UI-логика: Запрашиваем у пользователя файл(ы) снапшота
//     // // Используем m_root_path как стартовую директорию для удобства
//     // QStringList filePaths =
//     //     QFileDialog::getOpenFileNames(this,
//     //                                   tr("Выберите файлы снапшота (таймстепа)"),
//     //                                   m_root_path,
//     //                                   tr("Файлы симуляции (*.bin *.hdf5 *.csv *.dat);;Все файлы
//     (*.*)"));

//     // // Если пользователь отменил выбор, просто выходим
//     // if (filePaths.isEmpty()) {
//     //     return;
//     // }

//     // // Обновляем m_root_path путем последнего выбранного файла, чтобы при следующем открытии
//     // // диалог распахивался в этой же папке
//     // m_root_path = QFileInfo(filePaths.first()).absolutePath();

//     // // 2. Извлекаем базовое имя файла для названия узла в дереве
//     // // Например, если файл "snapshot_001.bin", имя будет "snapshot_001"
//     // QString snapshotName = QFileInfo(filePaths.first()).baseName();

//     // // 3. Бизнес-логика: Передаем задачу импорта в Ядро системы
//     // if (!m_app) {
//     //     return;
//     // }

//     // const auto dataController = m_app->dataController();
//     // if (dataController) {
//     //     // Используем метод импорта эксперимента.
//     //     // Если у вас один файл — ядро создаст для него узел (DataNode) внутри структуры,
//     //     // используя snapshotName как метку, и применит выбранную в комбобоксе версию формата.
//     //     dataController->importExperiment(snapshotName, filePaths, m_currentVersion);
//     // }
// }
void LayerExplorerWidget::handleAddExperiment() {
    // Запрашиваем директорию эксперимента
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Выберите папку эксперимента"), m_root_path);
    if (dirPath.isEmpty())
        return;

    if (m_app && m_app->dataController()) {
        m_app->dataController()->importExperiment(dirPath, m_currentVersion);
    }
}

void LayerExplorerWidget::handleRemoveElement() {
    QList<QUuid> ids = getSelectedIds();
    for (const QUuid& id : ids) {
        // Делегируем удаление через сигналы, удалит ноду из ObjectRegistry
        emit removalRequested(id);
    }
}

void LayerExplorerWidget::handleNodeSelected() {
    QList<QUuid> selectedIds = getSelectedIds();
    // Оповещаем мир о массовом изменении
    emit selectionChanged(selectedIds);
}
QList<QUuid> LayerExplorerWidget::getSelectedIds() const {
    QList<QUuid> ids;
    // Забираем выделенные индексы первой колонки
    auto indexes = ui->treeView_Layers->selectionModel()->selectedRows(0);

    for (const QModelIndex& index : indexes) {
        QUuid id = index.data(Models::DataTreeModel::CustomRoles::IdRole).toUuid();
        if (!id.isNull()) {
            ids.append(id);
        }
    }
    return ids;
}

// ---------------------------------------------------------
// @SECTION: Редактор слоев - Видимость слоев
// ---------------------------------------------------------

void LayerExplorerWidget::handleHideAll() {
    if (!m_treeModel)
        return;
    setCheckStateRecursiveInternal(QModelIndex(), Qt::Unchecked);
}

void LayerExplorerWidget::handleShowAll() {
    if (!m_treeModel)
        return;
    setCheckStateRecursiveInternal(QModelIndex(), Qt::Checked);
}

void LayerExplorerWidget::handleHideSelected() {
    if (!m_treeModel || !ui || !ui->treeView_Layers)
        return;
    auto* proxyModel = qobject_cast<QSortFilterProxyModel*>(ui->treeView_Layers->model());
    if (!proxyModel)
        return;
    QModelIndexList selectedProxyIndexes = ui->treeView_Layers->selectionModel()->selectedIndexes();
    for (const QModelIndex& proxyIndex : selectedProxyIndexes) {
        // Фильтруем по первой колонке, чтобы не обрабатывать одну строку несколько раз
        if (proxyIndex.column() != 0)
            continue;

        // Переводим прокси-индекс в индекс нашей m_treeModel
        QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);

        // Гасим выбранный элемент и всё, что находится внутри него
        setCheckStateRecursiveInternal(sourceIndex, Qt::Unchecked);
    }
}

void LayerExplorerWidget::handleShowSelected() {
    if (!m_treeModel || !ui || !ui->treeView_Layers)
        return;
    auto* proxyModel = qobject_cast<QSortFilterProxyModel*>(ui->treeView_Layers->model());
    if (!proxyModel)
        return;
    QModelIndexList selectedProxyIndexes = ui->treeView_Layers->selectionModel()->selectedIndexes();
    for (const QModelIndex& proxyIndex : selectedProxyIndexes) {
        // Фильтруем по первой колонке, чтобы не обрабатывать одну строку несколько раз
        if (proxyIndex.column() != 0)
            continue;

        // Переводим прокси-индекс в индекс нашей m_treeModel
        QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);

        // Гасим выбранный элемент и всё, что находится внутри него
        setCheckStateRecursiveInternal(sourceIndex, Qt::Checked);
    }
} // namespace QSpace::UI
void LayerExplorerWidget::handleHideUnselected() {
    if (!m_treeModel || !ui || !ui->treeView_Layers)
        return;

    auto* proxyModel = qobject_cast<QSortFilterProxyModel*>(ui->treeView_Layers->model());
    if (!proxyModel)
        return;

    // 1. Собираем ВСЕ выделенные прокси-индексы
    QModelIndexList selectedProxyIndexes = ui->treeView_Layers->selectionModel()->selectedIndexes();

    // 2. Переводим их в индексы m_treeModel и сохраняем в хэш-сет для быстрого поиска
    QSet<QModelIndex> selectedSourceIndexes;
    for (const QModelIndex& proxyIndex : selectedProxyIndexes) {
        if (proxyIndex.column() == 0) { // Нас интересует только первая колонка с чекбоксами
            selectedSourceIndexes.insert(proxyModel->mapToSource(proxyIndex));
        }
    }

    // 3. Запускаем рекурсивный поиск от корня дерева.
    // Всё, что не попало в selectedSourceIndexes, будет выключено.
    setCheckStateUnselectedRecursiveInternal(QModelIndex(), selectedSourceIndexes, Qt::Unchecked);
}

// ---------------------------------------------------------
// @SECTION: Редактор слоев - Фильтры слоев
// ---------------------------------------------------------
void LayerExplorerWidget::handleShowLoadedLayers(const bool isChecked) {
    Q_UNUSED(this);
    // Заглушка: Здесь можно реализовать проверку статуса ноды (загружена ли она в LRU кэш)
    // и скрывать/отображать соответствующие элементы QtreeView_Layers.
}
void LayerExplorerWidget::handleShowUnloadedLayers(const bool isChecked) {
    Q_UNUSED(this);

    // Заглушка: Аналогично для выгруженных слоев
}
void LayerExplorerWidget::handleFilterEquationToggled(const bool isChecked) {
    Q_UNUSED(this);

    // Активация фильтрации слоев на основе пользовательского уравнения/выражения
}
void LayerExplorerWidget::handleFilterEquationLineChange(const QString& line) {
    Q_UNUSED(this);
    // Обработка изменения выражения для фильтрации
}
// ---------------------------------------------------------
// @SECTION: Управление структурой
// ---------------------------------------------------------
void LayerExplorerWidget::handleExpandAll() {
    ui->treeView_Layers->expandAll();
}

void LayerExplorerWidget::handleCollapseAll() {
    ui->treeView_Layers->collapseAll();
}

void LayerExplorerWidget::handleCollapseAllFiles() {
    ui->treeViewFiles->collapseAll();
}

// ---------------------------------------------------------
// @SECTION: Проводник файлов
// ---------------------------------------------------------
void LayerExplorerWidget::handleRootPathChange(const QString& line) {
    QDir dir(line);
    if (dir.exists()) {
        m_root_path = line;
        if (auto* model = qobject_cast<QFileSystemModel*>(ui->treeViewFiles->model())) {
            ui->treeViewFiles->setRootIndex(model->index(m_root_path));
        }
    }
}
void LayerExplorerWidget::handleFindFile(const QString& line) {
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
// ---------------------------------------------------------
// @SECTION: Дополнительные методы
// ---------------------------------------------------------

void LayerExplorerWidget::setCheckStateRecursiveInternal(const QModelIndex& parentIndex,
                                                         Qt::CheckState     state) {
    if (!m_treeModel)
        return;
    int rows = m_treeModel->rowCount(parentIndex);
    for (int i = 0; i < rows; ++i) {
        QModelIndex childIndex = m_treeModel->index(i, 0, parentIndex);
        m_treeModel->setData(childIndex, state, Qt::CheckStateRole);
        // Рекурсивно для всех детей
        if (m_treeModel->hasChildren(childIndex)) {
            setCheckStateRecursiveInternal(childIndex, state);
        }
    }
}
void LayerExplorerWidget::setCheckStateUnselectedRecursiveInternal(
    const QModelIndex&       parentIndex,
    const QSet<QModelIndex>& selectedSourceIndexes,
    Qt::CheckState           state) {
    if (!m_treeModel)
        return;

    int rows = m_treeModel->rowCount(parentIndex);
    for (int i = 0; i < rows; ++i) {
        QModelIndex currentIndex = m_treeModel->index(i, 0, parentIndex);

        // Если этого элемента НЕТ в списке выделенных — меняем его состояние
        if (!selectedSourceIndexes.contains(currentIndex)) {
            m_treeModel->setData(currentIndex, state, Qt::CheckStateRole);
        }

        // Идем глубже по дереву (даже если родитель выделен, его дети могут быть не выделены)
        if (m_treeModel->hasChildren(currentIndex)) {
            setCheckStateUnselectedRecursiveInternal(currentIndex, selectedSourceIndexes, state);
        }
    }
}

void LayerExplorerWidget::handleShowCustomContextMenuForFile(const QPoint& pos) {
    // получаем индекс из модели файлов по позиции курсора
    QModelIndex index = ui->treeViewFiles->indexAt(pos);

    if (!index.isValid())
        return;

    // проверяем, что выбранный элемент это файл а не папка
    auto* model = qobject_cast<QFileSystemModel*>(ui->treeViewFiles->model());

    if (!model)
        return;

    if (model->isDir(index))
        return;

    QMenu    contextMenu(this);
    QAction* actionImport = contextMenu.addAction(tr("Загрузить файл"));
    connect(actionImport,
            &QAction::triggered,
            this,
            &LayerExplorerWidget::handleImportFileRequestFromFileExplorer);
    contextMenu.exec(ui->treeViewFiles->viewport()->mapToGlobal(pos));
}

} // namespace QSpace::UI
#include "LayerExplorerWidget.moc"