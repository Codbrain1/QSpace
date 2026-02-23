#pragma once
#include "Visualize/Renderer.h"
#include <QObject>
#include <map>
#include <memory>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>

namespace QSpace::Core {
class ViewManager : public QObject {
    Q_OBJECT
  public:
    explicit ViewManager(QObject* parent = nullptr);
    QUuid createView();
    // Создает окно с заданным ракурсом (удобно для Quad-View)
    QUuid createView(Visualize::CameraViewType type);

    Visualize::Renderer* getView(const QUuid& viewId);
    QUuid                getMainViewId() const;
    void                 removeView(const QUuid& id);

    // аргумент принимает указатель на Renderer
    template <typename Function> void forEachView(Function&& action) {
        for (auto& [id, renderer] : m_views) {
            action(renderer.get());
        }
    }
  signals:
    void viewCreated(QUuid id);
    void viewRemoved(QUuid id);

  private:
    std::map<QUuid, std::unique_ptr<Visualize::Renderer>> m_views;
    QUuid                                                 m_mainViewId;
};
} // namespace QSpace::Core
