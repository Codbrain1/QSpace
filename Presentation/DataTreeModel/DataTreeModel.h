#pragma once

#include <QAbstractItemModel>
#include <QMap>
#include <QSet>
#include <QUuid>
#include "Structures/CoreStructures.h"
#include <memory>

namespace QSpace::Core {
class ObjectRegistry;
class LayerManager;
class DataNode;
class DataContainer;
} // namespace QSpace::Core

namespace QSpace::Models {
enum class TreeMode { SnapShotView, ComponentView };

/**
 * @brief Внутренний узел дерева для маппинга QModelIndex в структуры ядра
 */
class DataTreeItem {
  public:
    // ДОБАВЛЕН ComponentGroup
    enum Type { Root, Experiment, Container, ComponentGroup, Node, LayerItem };

    // ДОБАВЛЕН параметр subType (по умолчанию -1)
    DataTreeItem(Type type, const QUuid& id, DataTreeItem* parent = nullptr, int subType = -1);
    ~DataTreeItem();

    Type type() const {
        return m_type;
    }

    QUuid id() const {
        return m_id;
    }

    int subType() const {
        return m_subType;
    } // Геттер для типа компоненты

    DataTreeItem* parent() const {
        return m_parent;
    }

    void          appendChild(DataTreeItem* child);
    DataTreeItem* child(int row) const;
    int           childCount() const;
    int           row() const;

  private:
    Type                 m_type;
    QUuid                m_id;
    int                  m_subType; // Хранит EntityType (0, 1, 2) для виртуальных групп
    DataTreeItem*        m_parent;
    QList<DataTreeItem*> m_children;
};

/**
 * @brief Модель древовидной структуры данных для отображения в QTreeView
 */
class DataTreeModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    enum Columns {
        NameColumn = 0, // Имя объекта + Чекбокс видимости
        TypeColumn,     // Тип данных (Gas, Stars, DarkMatter, Container)
        MemoryColumn,   // Статус в ОЗУ (Загружен / Выгружен LRU кэшем)
        SizeColumn,     // Количество частиц / дочерних элементов
        ColumnCount
    };

    explicit DataTreeModel(Core::ObjectRegistry* registry,
                           Core::LayerManager*   layerManager,
                           QObject*              parent = nullptr);
    ~DataTreeModel() override = default;

    void setTreeMode(TreeMode mode) {
        if (m_treeMode == mode) {
            return;
        }

        m_treeMode = mode;
        rebuildTree(); // Полностью пересобирает иерархию DataTreeItem'ов
    }

    // --- Реализация pure virtual методов QAbstractItemModel ---
    QModelIndex
    index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int         rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int         columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

  public slots:
    /**
     * @brief Полностью перестраивает внутренние индексы дерева на основе ObjectRegistry
     */
    void rebuildTree();

    /**
     * @brief Оповещает UI о том, что изменились данные ноды (например, LRU выгрузил её из памяти)
     */
    void refreshNode(const QUuid& nodeId);

  private:
    Core::ObjectRegistry* m_registry;
    Core::LayerManager*   m_layerManager;
    TreeMode              m_treeMode;

    std::unique_ptr<DataTreeItem> m_rootItem;
    QMap<QUuid, DataTreeItem*>    m_itemMap; // Быстрый доступ к узлам дерева по UUID
  private:
    void buildNodeBranch(std::shared_ptr<Core::DataNode> node, DataTreeItem* parentItem);
    Qt::CheckState
    calculateExperimentCheckState(std::shared_ptr<QSpace::Core::Experiment> exp) const;

    // Вспомогательные методы сборки дерева
    void buildContainerBranch(std::shared_ptr<Core::DataContainer> container,
                              DataTreeItem*                        parentItem);
    Qt::CheckState
            calculateContainerCheckState(std::shared_ptr<Core::DataContainer> container) const;
    QString entityTypeToString(int type) const;
    Qt::CheckState calculateGroupCheckState(DataTreeItem* groupItem) const;
};

} // namespace QSpace::Models