#pragma once
#include "Core/AppCore/AppCore.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Structures/CoreStructures.h"
#include <QObject>
#include <memory>
#include <qlist.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtreewidget.h>
#include <quuid.h>
namespace QSpace::UI {
class DataTreeController : public QObject {
    Q_OBJECT
  public:
    DataTreeController(Core::AppCore*                app,
                       QTreeWidget*                  tree,
                       QSpace::Core::ObjectRegistry* registry,
                       QObject*                      parent = nullptr);
    QList<QUuid> getSelectedIds() const;
  signals:
    void selectionChanged(const QList<QUuid>& selectedIds);
  private slots:
    void onNodeSelected();
    void onNodeAdded(std::shared_ptr<QSpace::Core::DataNode> node);
    void onNodeRemoved(const QUuid& id);
    void showContextMenu(const QPoint& pos);
    void onItemChahged(QTreeWidgetItem* item, int col);

  private:
    QTreeWidgetItem*              findTreeElementById(const QUuid& id);
    Core::AppCore*                m_app;
    QTreeWidget*                  m_tree;
    QSpace::Core::ObjectRegistry* m_registry;
};
} // namespace QSpace::UI