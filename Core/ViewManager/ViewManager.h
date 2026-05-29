#pragma once
#include "Interfaces/IView.h"
#include <QObject>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>
#include <vtkRenderWindow.h>
#include "Enums/RenderEnums.h"
#include <map>
#include <memory>

namespace QSpace::Core {
class ViewManager : public QObject {
    Q_OBJECT
  public:
    explicit ViewManager(QObject* parent = nullptr);
    // Создает окно с заданным ракурсом (удобно для Quad-View)
    QUuid createView(Visualize::ViewType type = Visualize::ViewType::VTK_3D);
    void  setMainView(const QUuid& viewId);

    std::shared_ptr<Visualize::IView> getView(const QUuid& viewId);
    QUuid                             getMainViewId() const;
    void                              removeView(const QUuid& id);

    // аргумент принимает указатель на IView
    template <typename Function> void forEachView(Function&& action) {
        for (auto& [id, renderer] : m_views) {
            action(renderer);
        }
    }

  signals:
    void viewCreated(const QUuid& id, Visualize::ViewType type);
    void viewRemoved(const QUuid& id, Visualize::ViewType type);
    void viewUpdateRequested(const QUuid& id);
    void allViewsUpdateRequested();

  private:
    std::map<QUuid, std::shared_ptr<Visualize::IView>> m_views;
    QUuid                                              m_mainViewId;
};
} // namespace QSpace::Core
