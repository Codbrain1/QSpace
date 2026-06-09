#include "DataTreeModel.h"
#include "Common/Structures/CoreStructures.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Enums/RenderEnums.h"

#include <QColor>
#include <QIcon>
#include <cstddef>
#include <memory>
#include <qnamespace.h>
#include <quuid.h>
#include <qvariant.h>

namespace QSpace::Models {

// ==========================================
//    РЕАЛИЗАЦИЯ ВСПОМОГАТЕЛЬНОГО DataTreeItem
// ==========================================
DataTreeItem::DataTreeItem(Type type, const QUuid& id, DataTreeItem* parent, Visualize::EntityType subType)
    : m_id(id), m_parent(parent), m_type(type), m_subType(subType) {
}

DataTreeItem* DataTreeItem::parent() const {
    return m_parent;
}

DataTreeItem::Type DataTreeItem::type() const {
    return m_type;
}

QUuid DataTreeItem::id() const {
    return m_id;
}

Visualize::EntityType DataTreeItem::subType() const {
    return m_subType;
} // Геттер для типа компоненты

void DataTreeItem::appendChild(std::unique_ptr<DataTreeItem>&& child) {
    m_children.emplace_back(std::move(child));
}

DataTreeItem* DataTreeItem::child(int row) const {
    return row >= 0 && row < childCount() ? m_children.at(row).get() : nullptr;
}

int DataTreeItem::childCount() const {
    return static_cast<int>(m_children.size());
}

int DataTreeItem::row() const {
    if (!m_parent)
        return 0;
    const auto& siblings = m_parent->m_children;
    for (size_t i = 0; i < siblings.size(); ++i) {
        if (siblings.at(i).get() == this) {
            return i;
        }
    }
    return -1;
}

// ==========================================
//    РЕАЛИЗАЦИЯ ОСНОВНОЙ DataTreeModel
// ==========================================

DataTreeModel::DataTreeModel(Core::ObjectRegistry* registry, Core::LayerManager* layerManager, QObject* parent)
    : QAbstractItemModel(parent), m_registry(registry), m_layerManager(layerManager) {
    m_rootItem = std::make_unique<DataTreeItem>(DataTreeItem::Root, QUuid());

    connect(m_registry, &Core::ObjectRegistry::experimentAdded, this, &DataTreeModel::rebuildTree);
    connect(m_registry, &Core::ObjectRegistry::snapshotAdded, this, &DataTreeModel::rebuildTree);
    connect(m_registry, &Core::ObjectRegistry::nodeAdded, this, &DataTreeModel::rebuildTree);
    connect(m_registry, &Core::ObjectRegistry::objectRemoved, this, &DataTreeModel::rebuildTree);
    connect(m_registry, &Core::ObjectRegistry::cleared, this, &DataTreeModel::rebuildTree);

    // Подключаем перерисовку статуса ОЗУ
    connect(m_registry, &Core::ObjectRegistry::nodeDataUpdated, this, &DataTreeModel::refreshNode);
    connect(m_layerManager, &Core::LayerManager::layerCreated, this, &DataTreeModel::rebuildTree);
    connect(m_layerManager, &Core::LayerManager::layerRemoved, this, &DataTreeModel::rebuildTree);

    rebuildTree();
}

void DataTreeModel::setTreeMode(TreeMode mode) {
    if (m_treeMode == mode) {
        return;
    }

    m_treeMode = mode;
    rebuildTree(); // Полностью пересобирает иерархию DataTreeItem'ов
}

void DataTreeModel::rebuildTree() {
    beginResetModel();
    m_itemMap.clear();
    m_rootItem = std::make_unique<DataTreeItem>(DataTreeItem::Root, QUuid());

    auto experiments = m_registry->getAllExperiments();

    // проходим по всем экспериментам
    for (const auto& exp : experiments) {
        if (!exp)
            continue;

        // создаем элемент эксперимента
        auto  expItem    = std::make_unique<DataTreeItem>(DataTreeItem::Experiment, exp->id, m_rootItem.get());
        auto* expItemPtr = expItem.get();
        m_itemMap.insert(exp->id, expItemPtr); // доабвляем указатель в дерево
        m_rootItem->appendChild(std::move(expItem));

        // ----- Два вида иерархии -----
        // 1) Временной: Эксперимент -> Снапшот -> Тип компоненты -> Слой
        if (m_treeMode == TreeMode::SnapShotView) {
            for (const auto& snap : exp->snapshots) {
                buildSnapshotBranch(snap, expItemPtr);
            }
        } // 2) Компонентный: Эксперимент -> Тип компоненты (Газ/Звезды/Темная материя) -> Нода -> Слой
        else if (m_treeMode == TreeMode::ComponentView) {
            // 1. Группируем ноды со всех снапшотов по их физическому типу (EntityType)
            QMap<Visualize::EntityType, QList<std::shared_ptr<Core::DataNode>>> groupedNodesByEntityType;
            for (const auto& snap : exp->snapshots) {
                if (!snap)
                    continue;
                for (const auto& node : snap->components) {
                    if (node) {
                        groupedNodesByEntityType[node->type].append(node);
                    }
                }
            }

            // 2. Создаем ветку для каждой уникальной группы
            for (const auto& [entityType, nodes] : groupedNodesByEntityType.asKeyValueRange()) {
                buildComponentBranch(entityType, nodes, expItemPtr);
            }
        }
    }
    endResetModel();
}

// ====== Создает временную структуру ======
void DataTreeModel::buildSnapshotBranch(std::shared_ptr<Core::Snapshot> snapshot, DataTreeItem* parentItem) {
    if (!snapshot)
        return;

    // 1. Создаем и добавляем элемент контейнера (Снапшота)
    auto  snapshotItem    = std::make_unique<DataTreeItem>(DataTreeItem::Snapshot, snapshot->id, parentItem);
    auto* snapshotItemPtr = snapshotItem.get();
    m_itemMap.insert(snapshot->id, snapshotItemPtr);
    parentItem->appendChild(std::move(snapshotItem));

    // 2. Добавляем ноды (физические компоненты: газ, звезды...)
    for (const auto& node : snapshot->components) {
        if (!node)
            continue;

        buildNodeBranch(node, snapshotItemPtr);
    }
}

// ====== Создает компонентную структуру ======
void DataTreeModel::buildComponentBranch(const Visualize::EntityType&                  entityType,
                                         const QList<std::shared_ptr<Core::DataNode>>& nodes,
                                         DataTreeItem*                                 parentItem) {
    // Генерируем виртуальный UUID для группы, чтобы TreeView мог её идентифицировать
    QUuid componentId      = QUuid::createUuidV5(parentItem->id(), QString::number(static_cast<int>(entityType)));
    auto  componentItem    = std::make_unique<DataTreeItem>(DataTreeItem::ComponentGroup, componentId, parentItem, entityType);
    auto* componentItemPtr = componentItem.get();
    m_itemMap.insert(componentId, componentItemPtr);
    parentItem->appendChild(std::move(componentItem));

    // Добавляем ноды в эту группу
    for (const auto& node : nodes) {
        buildNodeBranch(node, componentItemPtr);
    }
}

// ====== Отображение отдельного узла данных и его представлений (слоев)
void DataTreeModel::buildNodeBranch(std::shared_ptr<Core::DataNode> node, DataTreeItem* parentItem) {
    auto  nodeItem    = std::make_unique<DataTreeItem>(DataTreeItem::DataNode, node->id, parentItem, node->type);
    auto* nodeItemPtr = nodeItem.get();
    m_itemMap.insert(node->id, nodeItemPtr);
    parentItem->appendChild(std::move(nodeItem));

    // 3. Добавляем слои визуализации, которые относятся к этой конкретной ноде
    auto nodeLayers = m_layerManager->getLayersForNode(node->id);
    for (const auto& layer : nodeLayers) {
        if (!layer)
            continue;

        // Используем ID слоя, чтобы потом в методе data() вытащить настройки конкретного окна
        auto  layerItem    = std::make_unique<DataTreeItem>(DataTreeItem::LayerItem, layer->layerId, nodeItemPtr);
        auto* layerItemPtr = layerItem.get();
        nodeItemPtr->appendChild(std::move(layerItem));
        m_itemMap.insert(layer->layerId, layerItemPtr);
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

    DataTreeItem* parentItem = parent.isValid() ? static_cast<DataTreeItem*>(parent.internalPointer()) : m_rootItem.get();

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

    DataTreeItem* parentItem = parent.isValid() ? static_cast<DataTreeItem*>(parent.internalPointer()) : m_rootItem.get();

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

    auto*                  item   = static_cast<DataTreeItem*>(index.internalPointer());
    DataTreeModel::Columns column = static_cast<DataTreeModel::Columns>(index.column());

    // =========================================================================
    // 1. РОЛЬ: Состояние чекбокса видимости (Только для первой колонки NameColumn)
    // =========================================================================
    if (role == Qt::CheckStateRole && column == Columns::NameColumn) {
        switch (item->type()) {
            case DataTreeItem::Type::LayerItem: {
                auto layer = m_layerManager->getLayer(item->id());
                return (layer && layer->settings->isVisible) ? Qt::Checked : Qt::Unchecked;
            }
            case DataTreeItem::Type::DataNode: {
                auto node = m_registry->getNode(item->id());
                return (node && node->masterSettings->isVisible) ? Qt::Checked : Qt::Unchecked;
            }
            case DataTreeItem::Type::ComponentGroup: {
                return calculateComponentGroupCheckState(item);
            }
            case DataTreeItem::Type::Snapshot: {
                auto snapshot = m_registry->getSnapshot(item->id());
                return calculateSnapshotheckState(snapshot);
            }
            case DataTreeItem::Type::Experiment: {
                auto experiment = m_registry->getExperiment(item->id());
                return calculateExperimentCheckState(experiment);
            }
            default:
                return Qt::Unchecked;
        }
    }
    // =========================================================================
    // 2. РОЛЬ: Отображение текста в ячейках таблицы
    // =========================================================================
    if (role == Qt::DisplayRole) {
        switch (item->type()) {
            case DataTreeItem::Type::LayerItem: {
                auto layer = m_layerManager->getLayer(item->id());
                if (!layer)
                    return QVariant();

                switch (column) {
                    case NameColumn:
                        return QString("Layer: %1").arg(layer->name);
                    // case TypeColumn:
                    //     return "Visualization Layer";
                    default:
                        return "";
                }
            }
            case DataTreeItem::Type::DataNode: {
                auto node = m_registry->getNode(item->id());
                if (!node)
                    return QVariant();
                switch (column) {
                    case DataTreeModel::Columns::NameColumn: {
                        if (m_treeMode == TreeMode::SnapShotView)
                            return entityTypeToString(node->type);
                        else if (m_treeMode == TreeMode::ComponentView)
                            return node->label;
                        else
                            return "ERROR";
                    }
                    // case DataTreeModel::Columns::TypeColumn:
                    //     return entityTypeToString(node->type);
                    case DataTreeModel::Columns::MemoryColumn:
                        return (node->data != nullptr) ? "In RAM" : "Unloaded";
                    case DataTreeModel::Columns::SizeColumn:
                        return QString::number(node->stats.pointCount);
                    default:
                        return "";
                }
            }
            case DataTreeItem::Type::ComponentGroup: {
                switch (column) {
                    case DataTreeModel::Columns::NameColumn:
                        return entityTypeToString(item->subType());
                    // case DataTreeModel::Columns::TypeColumn:
                    //     return "Component collection";
                    case DataTreeModel::SizeColumn:
                        return QString("[%1 nodes]").arg(item->childCount());
                    default:
                        return "";
                }
            }
            case DataTreeItem::Type::Snapshot: {
                auto snapshot = m_registry->getSnapshot(item->id());
                if (!snapshot)
                    return QVariant();
                switch (column) {
                    case DataTreeModel::Columns::NameColumn:
                        return snapshot->name;
                    case DataTreeModel::Columns::SizeColumn:
                        return QString("[%1 components]").arg(snapshot->components.size());
                    default:
                        return "";
                }
            }
            case DataTreeItem::Type::Experiment: {
                auto experiment = m_registry->getExperiment(item->id());
                if (!experiment)
                    return QVariant();
                switch (column) {
                    case DataTreeModel::Columns::NameColumn:
                        return experiment->name;
                    default:
                        return "";
                }
            }
            default:
                return QVariant();
        }
    }

    // =========================================================================
    // 3. РОЛЬ: Стилизация интерфейса (Кастомизация отображения)
    // =========================================================================
    if (role == Qt::ForegroundRole && item->type() == DataTreeItem::DataNode) {
        auto node = m_registry->getNode(item->id());
        // Если данные выгружены LRU-кэшем — приглушаем текст серым цветом
        if (node && node->data == nullptr) {
            return QColor(Qt::gray);
        }
    }

    // Для всех необработанных ролей возвращаем пустой QVariant (Qt применит дефолты)
    return QVariant();
} // namespace QSpace::Models

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
            if (child->type() == DataTreeItem::DataNode) {
                auto node = m_registry->getNode(child->id());
                if (node) {
                    node->masterSettings->isVisible = visible;
                    m_layerManager->updateNodeMasterSettings(node->id); // Каскадно скроет слои
                }
            }
        }
    } else if (item->type() == DataTreeItem::Snapshot) {
        auto container = m_registry->getSnapshot(item->id());
        if (container)
            m_layerManager->setContainerVisibility(container, visible);
    } else if (item->type() == DataTreeItem::DataNode) {
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

    emit dataChanged(index, index, {Qt::CheckStateRole});
    return true;
}

// --- Оформление шапки таблицы ---

QVariant DataTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case NameColumn:
                return "Name";
            // case TypeColumn:
            //     return "Physical Type";
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
Qt::CheckState DataTreeModel::calculateComponentGroupCheckState(DataTreeItem* groupItem) const {
    if (!groupItem || groupItem->childCount() == 0)
        return Qt::Unchecked;

    int total        = groupItem->childCount();
    int visibleCount = 0;

    for (int i = 0; i < total; ++i) {
        DataTreeItem* child = groupItem->child(i);
        if (child->type() == DataTreeItem::DataNode) {
            auto node = m_registry->getNode(child->id());
            if (node && node->masterSettings->isVisible) {
                visibleCount++;
            }
        }
        // calculateComponentGroupCheckState(child);
    }

    if (visibleCount == 0)
        return Qt::Unchecked;
    if (visibleCount == total)
        return Qt::Checked;
    return Qt::PartiallyChecked;
}

Qt::CheckState DataTreeModel::calculateExperimentCheckState(std::shared_ptr<Core::Experiment> exp) const {
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

Qt::CheckState DataTreeModel::calculateSnapshotheckState(std::shared_ptr<Core::Snapshot> container) const {
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

QString DataTreeModel::entityTypeToString(Visualize::EntityType type) const {
    // Маппинг вашего перечисления EntityType (Газ, Звезды, Темная Материя) в красивый текст
    switch (type) {
        case Visualize::EntityType::Gas:
            return "Gas / SPH";
        case Visualize::EntityType::Stars:
            return "Stars";
        case Visualize::EntityType::DarkMatter:
            return "Dark Matter";
        default:
            return "Unknown";
    }
}

} // namespace QSpace::Models