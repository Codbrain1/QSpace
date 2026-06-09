#pragma once

#include "Enums/RenderEnums.h"
#include "Structures/CoreStructures.h"
#include <QAbstractItemModel>
#include <QMap>
#include <QSet>
#include <QUuid>
#include <memory>

namespace QSpace::Core {
class ObjectRegistry;
class LayerManager;
class DataNode;
class Snapshot;
} // namespace QSpace::Core

namespace QSpace::Models {
enum class TreeMode { SnapShotView, ComponentView };

/**
 * @brief Внутренний узел дерева для маппинга QModelIndex в структуры ядра
 */
class DataTreeItem {
  public:
    enum Type { Root, Experiment, Snapshot, ComponentGroup, DataNode, LayerItem };

    //  параметр subType (по умолчанию -1)
    explicit DataTreeItem(Type type, const QUuid& id, DataTreeItem* parent = nullptr, Visualize::EntityType subType = Visualize::EntityType::Unknown);
    ~DataTreeItem() = default;
    Type                  type() const;
    QUuid                 id() const;
    Visualize::EntityType subType() const;

    DataTreeItem* parent() const;

    void          appendChild(std::unique_ptr<DataTreeItem>&& child);
    DataTreeItem* child(int row) const;
    int           childCount() const;

    int row() const;

  private:
    std::vector<std::unique_ptr<DataTreeItem>> m_children;
    QUuid                                      m_id; // UUid записи данных в реестру
    DataTreeItem*                              m_parent;
    Type                                       m_type;    // тип элемента (эксперимент, снапшот или слой)
    Visualize::EntityType                      m_subType; // Хранит EntityType (0, 1, 2) для виртуальных групп
};

/**
 * @brief Модель древовидной структуры данных для отображения в QTreeView
 */
class DataTreeModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    enum Columns {
        NameColumn = 0, // Имя объекта + Чекбокс видимости
        // TypeColumn,     // Тип данных (Gas, Stars, DarkMatter, Container)
        MemoryColumn, // Статус в ОЗУ (Загружен / Выгружен LRU кэшем)
        SizeColumn,   // Количество частиц / дочерних элементов
        ColumnCount
    };

    explicit DataTreeModel(Core::ObjectRegistry* registry, Core::LayerManager* layerManager, QObject* parent = nullptr);
    ~DataTreeModel() override = default;

    void setTreeMode(TreeMode mode);

    // --- Реализация pure virtual методов QAbstractItemModel ---
    // используются для навигации по дереву
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;

    // задают размеры таблицы
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    // вызывается для отрисовки таблицы
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // реакция на действия пользователя (например, клик по чекбоксу)
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override; // TODO
    // возвращает заголовки столбцов
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    // управляет интерактивностью
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
    TreeMode              m_treeMode = TreeMode::SnapShotView;

    std::unique_ptr<DataTreeItem> m_rootItem;
    QMap<QUuid, DataTreeItem*>    m_itemMap; // Быстрый доступ к узлам дерева по UUID

    // Вспомогательные методы сборки дерева
    void buildSnapshotBranch(std::shared_ptr<Core::Snapshot> snapshot, DataTreeItem* parentItem);
    void buildComponentBranch(const Visualize::EntityType& entityType, const QList<std::shared_ptr<Core::DataNode>>& nodes, DataTreeItem* parentItem);
    void buildNodeBranch(std::shared_ptr<Core::DataNode> node, DataTreeItem* parentItem);

    Qt::CheckState calculateExperimentCheckState(std::shared_ptr<QSpace::Core::Experiment> exp) const; // TODO
    Qt::CheckState calculateSnapshotheckState(std::shared_ptr<Core::Snapshot> container) const;        // TODO
    Qt::CheckState calculateComponentGroupCheckState(DataTreeItem* groupItem) const;                   // TODO
    QString        entityTypeToString(Visualize::EntityType type) const;                               // TODO
};

} // namespace QSpace::Models