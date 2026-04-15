#pragma once
#include "Core/AppCore/AppCore.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Structures/CoreStructures.h"
#include <QObject>
#include <functional>
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
    DataTreeController(QTreeWidget* tree, QObject* parent = nullptr);
    QList<QUuid> getSelectedIds() const;
  signals:
    void selectionChanged(const QList<QUuid>& selectedIds);
    void removalRequested(const QUuid& id);
    void updateNodeSettingsRequested(const QUuid&                               id,
                                     std::function<void(Core::VisualSettings&)> modifer);
  public slots:
    void onNodeAdded(std::shared_ptr<QSpace::Core::DataNode> node);
    void onObjectRemoved(const QUuid& id);

  private slots:
    void onNodeSelected();
    void showContextMenu(const QPoint& pos);
    void onItemChanged(QTreeWidgetItem* item, int col);
    // void onActionOpenVisualSettingsTrigered();

  private:
    QTreeWidgetItem* findTreeElementById(const QUuid& id);
    QTreeWidget*     m_tree;
};
} // namespace QSpace::UI