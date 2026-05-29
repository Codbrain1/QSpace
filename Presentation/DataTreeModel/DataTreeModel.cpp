#include "DataTreeModel.h"
#include "Common/Structures/CoreStructures.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"


#include <QColor>
#include <QIcon>
#include <quuid.h>

namespace QSpace::Models {

// ==========================================
//    РЕАЛИЗАЦИЯ ВСПОМОГАТЕЛЬНОГО DataTreeItem
// ==========================================
DataTreeItem::DataTreeItem(Type type, const QUuid& id, DataTreeItem* parent, int subType)
    : m_type(type), m_id(id), m_parent(parent), m_subType(subType) {
}

DataTreeItem::~DataTreeItem() {
    qDeleteAll(m_children);
}

void DataTreeItem::appendChild(DataTreeItem* child) {
    m_children.append(child);
}

DataTreeItem* DataTreeItem::child(int row) const {
    return m_children.value(row, nullptr);
}

int DataTreeItem::childCount() const {
    return m_children.count();
}

int DataTreeItem::row() const {
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<DataTreeItem*>(this));
    }
    return 0;
}

// ==========================================
//    РЕАЛИЗАЦИЯ ОСНОВНОЙ DataTreeModel
// ==========================================

DataTreeModel::DataTreeModel(Core::ObjectRegistry* registry,
                             Core::LayerManager*   layerManager,
                             QObject*              parent)
    : QAbstractItemModel(parent), m_registry(registry), m_layerManager(layerManager) {
    m_rootItem = std::make_unique<DataTreeItem>(DataTreeItem::Root, QUuid());

    connect(m_registry, &Core::ObjectRegistry::experimentAdded, this, &DataTreeModel::rebuildTree);
    connect(m_registry, &Core::ObjectRegistry::containerAdded, this, &DataTreeModel::rebuildTree);
    connect(m_registry, &Core::ObjectRegistry::nodeAdded, this, &DataTreeModel::rebuildTree);
    connect(m_registry, &Core::ObjectRegistry::objectRemoved, this, &DataTreeModel::rebuildTree);
    connect(m_registry, &Core::ObjectRegistry::cleared, this, &DataTreeModel::rebuildTree);

    // Подключаем перерисовку статуса ОЗУ
    connect(m_registry, &Core::ObjectRegistry::nodeDataUpdated, this, &DataTreeModel::refreshNode);
    connect(m_layerManager, &Core::LayerManager::layerCreated, this, &DataTreeModel::rebuildTree);
    connect(m_layerManager, &Core::LayerManager::layerRemoved, this, &DataTreeModel::rebuildTree);

    rebuildTree();
}

void DataTreeModel::rebuildTree() {
    beginResetModel();
    m_itemMap.clear();
    m_rootItem = std::make_unique<DataTreeItem>(DataTreeItem::Root, QUuid());

    auto experiments = m_registry->getAllExperiments();

    for (const auto& exp : experiments) {
        if (!exp)
            continue;

        auto* expItem = new DataTreeItem(DataTreeItem::Experiment, exp->id, m_rootItem.get());
        m_rootItem->appendChild(expItem);
        m_itemMap.insert(exp->id, expItem);

        if (m_treeMode == TreeMode::SnapShotView) {
            // ИЕРАРХИЯ: Эксперимент -> Снапшот -> Нода -> Слой
            for (const auto& snap : exp->snapshots) {
                buildContainerBranch(snap, expItem);
            }
        } else if (m_treeMode == TreeMode::ComponentView) {
            // ИЕРАРХИЯ: Эксперимент -> Группа компонент (Газ/ТМ/Звезды) -> Нода -> Слой

            // 1. Группируем ноды со всех снапшотов по их физическому типу (EntityType)
            QMap<int, QList<std::shared_ptr<Core::DataNode>>> groupedNodes;
            for (const auto& snap : exp->snapshots) {
                if (!snap)
                    continue;
                for (const auto& node : snap->components) {
                    if (node) {
                        groupedNodes[static_cast<int>(node->type)].append(node);
                    }
                }
            }

            // 2. Создаем ветку для каждой уникальной группы
            for (auto it = groupedNodes.begin(); it != groupedNodes.end(); ++it) {
                int entityTypeInt = it.key();

                // Генерируем виртуальный UUID для группы, чтобы TreeView мог её идентифицировать
                QUuid groupId = QUuid::createUuid();
                auto* groupItem =
                    new DataTreeItem(DataTreeItem::ComponentGroup, groupId, expItem, entityTypeInt);
                expItem->appendChild(groupItem);
                m_itemMap.insert(groupId, groupItem);

                // Добавляем ноды в эту группу
                for (const auto& node : it.value()) {
                    buildNodeBranch(node, groupItem);
                }
            }
        }
    }
    endResetModel();
}

void DataTreeModel::buildContainerBranch(std::shared_ptr<Core::DataContainer> container,
                                         DataTreeItem*                        parentItem) {
    if (!container)
        return;

    // 1. Создаем и добавляем элемент контейнера (Снапшота)
    auto* containerItem = new DataTreeItem(DataTreeItem::Container, container->id, parentItem);
    parentItem->appendChild(containerItem);
    m_itemMap.insert(container->id, containerItem);

    // 2. Добавляем ноды (физические компоненты: газ, звезды...)
    for (const auto& node : container->components) {
        if (!node)
            continue;

        auto* nodeItem = new DataTreeItem(DataTreeItem::Node, node->id, containerItem);
        containerItem->appendChild(nodeItem);
        m_itemMap.insert(node->id, nodeItem);

        // 3. Добавляем слои визуализации, которые относятся к этой конкретной ноде
        auto nodeLayers = m_layerManager->getLayersForNode(node->id);
        for (const auto& layer : nodeLayers) {
            if (!layer)
                continue;

            // Используем ID слоя, чтобы потом в методе data() вытащить настройки конкретного окна
            auto* layerItem = new DataTreeItem(DataTreeItem::LayerItem, layer->layerId, nodeItem);
            nodeItem->appendChild(layerItem);
            m_itemMap.insert(layer->layerId, layerItem);
        }
    }
}

void DataTreeModel::refreshNode(const QUuid& nodeId) {
    if (!m_itemMap.contains(nodeId))
        return;

    DataTreeItem* item        = m_itemMap.value(nodeId);
    QModelIndex   topLeft     = createIndex(item->row(), 0, item);
    QModelIndex   bottomRight = createIndex(item->row(), ColumnCount - 1, item);

    emit dataChanged(topLeft, bottomRight);
}

// --- Навигационные методы дерева Qt ---

QModelIndex DataTreeModel::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DataTreeItem* parentItem =
        parent.isValid() ? static_cast<DataTreeItem*>(parent.internalPointer()) : m_rootItem.get();

    DataTreeItem* childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex DataTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid())
        return QModelIndex();

    auto*         childItem  = static_cast<DataTreeItem*>(child.internalPointer());
    DataTreeItem* parentItem = childItem->parent();

    if (parentItem == m_rootItem.get() || !parentItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int DataTreeModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0)
        return 0;

    DataTreeItem* parentItem =
        parent.isValid() ? static_cast<DataTreeItem*>(parent.internalPointer()) : m_rootItem.get();

    return parentItem->childCount();
}

int DataTreeModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return ColumnCount;
}

// --- Чтение и вывод данных в UI ---
QVariant DataTreeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();
    auto* item = static_cast<DataTreeItem*>(index.internalPointer());

    if (role == Qt::CheckStateRole && index.column() == NameColumn) {
        if (item->type() == DataTreeItem::Experiment) {
            auto exp = m_registry->getExperiment(item->id());
            return exp ? calculateExperimentCheckState(exp) : Qt::Unchecked;
        } else if (item->type() == DataTreeItem::ComponentGroup) {
            return calculateGroupCheckState(item); // Чекбокс группы Газа/Звезд
        } else if (item->type() == DataTreeItem::Container) {
            auto container = m_registry->getContainer(item->id());
            return container ? calculateContainerCheckState(container) : Qt::Unchecked;
        } else if (item->type() == DataTreeItem::Node) {
            auto node = m_registry->getNode(item->id());
            return (node && node->masterSettings->isVisible) ? Qt::Checked : Qt::Unchecked;
        } else if (item->type() == DataTreeItem::LayerItem) {
            auto layer = m_layerManager->getLayer(item->id());
            return (layer && layer->settings->isVisible) ? Qt::Checked : Qt::Unchecked;
        }
    }

    if (role == Qt::DisplayRole) {
        if (item->type() == DataTreeItem::LayerItem) {
            auto layer = m_layerManager->getLayer(item->id());
            if (!layer)
                return QVariant();

            switch (index.column()) {
                case NameColumn:
                    return QString("Layer: %1").arg(layer->name);
                case TypeColumn:
                    return "Visualization Layer";
                case MemoryColumn:
                    return "";
                case SizeColumn:
                    return "";
            }
        } else if (item->type() == DataTreeItem::Node) {
            auto node = m_registry->getNode(item->id());
            if (!node)
                return QVariant();

            switch (index.column()) {
                case NameColumn:
                    return node->label;
                case TypeColumn:
                    return entityTypeToString(static_cast<int>(node->type));
                case MemoryColumn:
                    return (node->data != nullptr) ? "In RAM" : "Swapped (LRU)";
                case SizeColumn:
                    return QString::number(node->stats.pointCount);
            }
        } else if (item->type() == DataTreeItem::Container) {
            auto container = m_registry->getContainer(item->id());
            if (!container)
                return QVariant();

            switch (index.column()) {
                case NameColumn:
                    return container->name;
                case TypeColumn:
                    return "Snapshot";
                case MemoryColumn:
                    return "";
                case SizeColumn:
                    return QString("[%1 elements]")
                        .arg(container->components.size() + container->components.size());
            }
        } else if (item->type() == DataTreeItem::ComponentGroup) {
            switch (index.column()) {
                case NameColumn:
                    return entityTypeToString(item->subType()) + " Group";
                case TypeColumn:
                    return "Component Collection";
                case MemoryColumn:
                    return "";
                case SizeColumn:
                    return QString("[%1 timeslices]").arg(item->childCount());
            }
        }
    }

    // Небольшие UX-улучшения: подсветка выгруженных файлов и иконки
    if (role == Qt::ForegroundRole && item->type() == DataTreeItem::Node) {
        auto node = m_registry->getNode(item->id());
        if (node && node->data == nullptr) {
            return QColor(Qt::gray); // Выгруженные LRU-кэшем файлы делаем серыми
        }
    }

    return QVariant();
}

// --- Обработка клика по чекбоксу (Изменение видимости) ---
bool DataTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || role != Qt::CheckStateRole || index.column() != NameColumn)
        return false;

    auto* item    = static_cast<DataTreeItem*>(index.internalPointer());
    bool  visible = (value.toInt() == Qt::Checked);

    if (item->type() == DataTreeItem::Experiment) {
        auto exp = m_registry->getExperiment(item->id());
        if (exp) {
            for (auto& snap : exp->snapshots)
                m_layerManager->setContainerVisibility(snap, visible);
        }
    } else if (item->type() == DataTreeItem::ComponentGroup) {
        // Проходимся по всем нодам этой группы (например, выключаем ВЕСЬ газ во всех шагах)
        for (int i = 0; i < item->childCount(); ++i) {
            DataTreeItem* child = item->child(i);
            if (child->type() == DataTreeItem::Node) {
                auto node = m_registry->getNode(child->id());
                if (node) {
                    node->masterSettings->isVisible = visible;
                    m_layerManager->updateNodeMasterSettings(node->id); // Каскадно скроет слои
                }
            }
        }
    } else if (item->type() == DataTreeItem::Container) {
        auto container = m_registry->getContainer(item->id());
        if (container)
            m_layerManager->setContainerVisibility(container, visible);
    } else if (item->type() == DataTreeItem::Node) {
        auto node = m_registry->getNode(item->id());
        if (node) {
            node->masterSettings->isVisible = visible;
            m_layerManager->updateNodeMasterSettings(node->id);
        }
    } else if (item->type() == DataTreeItem::LayerItem) {
        auto layer = m_layerManager->getLayer(item->id());
        if (layer) {
            layer->settings->isVisible = visible;
            layer->update();
        }
    }

    emit dataChanged(QModelIndex(), QModelIndex());
    return true;
}

// --- Оформление шапки таблицы ---

QVariant DataTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case NameColumn:
                return "Component/Snapshot";
            case TypeColumn:
                return "Physical Type";
            case MemoryColumn:
                return "RAM Status";
            case SizeColumn:
                return "Particles Count";
        }
    }
    return QVariant();
}

Qt::ItemFlags DataTreeModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // Делаем интерактивным чекбокс только в первой колонке
    if (index.column() == NameColumn) {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

// --- Внутренние утилиты калькуляции состояний ---
Qt::CheckState DataTreeModel::calculateGroupCheckState(DataTreeItem* groupItem) const {
    if (!groupItem || groupItem->childCount() == 0)
        return Qt::Unchecked;

    int total        = groupItem->childCount();
    int visibleCount = 0;

    for (int i = 0; i < total; ++i) {
        DataTreeItem* child = groupItem->child(i);
        if (child->type() == DataTreeItem::Node) {
            auto node = m_registry->getNode(child->id());
            if (node && node->masterSettings->isVisible) {
                visibleCount++;
            }
        }
    }

    if (visibleCount == 0)
        return Qt::Unchecked;
    if (visibleCount == total)
        return Qt::Checked;
    return Qt::PartiallyChecked;
}

Qt::CheckState
DataTreeModel::calculateExperimentCheckState(std::shared_ptr<Core::Experiment> exp) const {
    // Этот метод универсален и работает независимо от текущего m_treeMode,
    // так как опрашивает сам реестр, а не визуальное дерево.
    if (!exp)
        return Qt::Unchecked;

    int total        = 0;
    int visibleCount = 0;

    for (const auto& snap : exp->snapshots) {
        if (!snap)
            continue;
        for (const auto& node : snap->components) {
            if (!node)
                continue;
            total++;
            if (node->masterSettings->isVisible) {
                visibleCount++;
            }
        }
    }

    if (total == 0 || visibleCount == 0)
        return Qt::Unchecked;
    if (visibleCount == total)
        return Qt::Checked;
    return Qt::PartiallyChecked;
}

Qt::CheckState
DataTreeModel::calculateContainerCheckState(std::shared_ptr<Core::DataContainer> container) const {
    if (!container || container->components.isEmpty()) {
        return Qt::Unchecked;
    }

    int total        = 0;
    int visibleCount = 0;

    // Считаем только атомарные ноды (Газ, ТМ, Звезды) внутри данного снапшота
    for (const auto& node : container->components) {
        if (!node)
            continue;

        total++;
        if (node->masterSettings->isVisible) {
            visibleCount++;
        }
    }

    // Финальный расчет состояния чекбокса для снапшота
    if (total == 0 || visibleCount == 0) {
        return Qt::Unchecked;
    }
    if (visibleCount == total) {
        return Qt::Checked;
    }

    return Qt::PartiallyChecked;
}

QString DataTreeModel::entityTypeToString(int type) const {
    // Маппинг вашего перечисления EntityType (Газ, Звезды, Темная Материя) в красивый текст
    switch (type) {
        case 0:
            return "Gas / SPH";
        case 1:
            return "Stars";
        case 2:
            return "Dark Matter";
        default:
            return "Particles Data";
    }
}

} // namespace QSpace::Models