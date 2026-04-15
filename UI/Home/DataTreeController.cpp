#include "DataTreeController.h"
#include "Core/AppCore/AppCore.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Structures/CoreStructures.h"
#include <QMenu>
#include <memory>
#include <qaction.h>
#include <qcontainerfwd.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpoint.h>
#include <qtreewidget.h>
#include <quuid.h>

namespace QSpace::UI {
DataTreeController::DataTreeController(QTreeWidget* tree, QObject* parent)
    : QObject(parent), m_tree(tree) {
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree,
            &QTreeWidget::customContextMenuRequested,
            this,
            &DataTreeController::showContextMenu);
    connect(m_tree, &QTreeWidget::itemChanged, this, &DataTreeController::onItemChanged);
    connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &DataTreeController::onNodeSelected);
}
void DataTreeController::onNodeSelected() {
    QList<QUuid> selectedIds = getSelectedIds();
    // Оповещаем мир о массовом изменении
    emit selectionChanged(selectedIds);
}
QList<QUuid> DataTreeController::getSelectedIds() const {
    QList<QUuid> ids;
    auto         items = m_tree->selectedItems();

    for (auto* item : items) {
        QString idStr = item->data(0, Qt::UserRole).toString();
        if (!idStr.isEmpty()) {
            ids.append(QUuid::fromString(idStr));
        }
    }
    return ids;
}
void DataTreeController::onNodeAdded(std::shared_ptr<QSpace::Core::DataNode> node) {
    m_tree->blockSignals(true);
    auto* item = new QTreeWidgetItem(m_tree);
    item->setText(0, node->label);
    item->setData(0, Qt::UserRole, node->id.toString());
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, node->settings.isVisible ? Qt::Checked : Qt::Unchecked);
    m_tree->blockSignals(false);
}
void DataTreeController::onObjectRemoved(const QUuid& id) {
    QTreeWidgetItem* item = findTreeElementById(id);
    if (item) {
        delete item;
    }
}
void DataTreeController::showContextMenu(const QPoint& pos) {
    // находим элемент по позиции
    QTreeWidgetItem* item = m_tree->itemAt(pos);
    if (!item)
        return;
    QUuid id = QUuid::fromString(item->data(0, Qt::UserRole).toString());
    // создаем меню в данном месте
    QMenu    menu;
    QAction* action = menu.addAction(tr("delete layer"));
    // QAction* action1 = menu.addAction((tr("open visual properties")));

    // если пользователь нажмет удалить то будет вызвано действие удаления объекта
    connect(action, &QAction::triggered, this, [this, &id]() { emit removalRequested(id); });

    menu.exec(m_tree->viewport()->mapToGlobal(pos));
}
void DataTreeController::onItemChanged(QTreeWidgetItem* item, int col) {
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
QTreeWidgetItem* DataTreeController::findTreeElementById(const QUuid& id) {
    QString idStr = id.toString();
    // Проходим по всем элементам верхнего уровня
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* topItem = m_tree->topLevelItem(i);
        if (topItem->data(0, Qt::UserRole).toString() == idStr) {
            return topItem;
        }

        // Ищем в детях (если это контейнер/группа)
        for (int j = 0; j < topItem->childCount(); ++j) {
            QTreeWidgetItem* child = topItem->child(j);
            if (child->data(0, Qt::UserRole).toString() == idStr) {
                return child;
            }
        }
    }
    return nullptr;
}
} // namespace QSpace::UI