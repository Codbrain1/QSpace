#pragma once
#include "Enums/RenderEnums.h"
#include "Interfaces/IView.h"
#include <QObject>
#include <map>
#include <memory>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>
#include <vtkRenderWindow.h>

namespace QSpace::Core {
class ViewManager : public QObject {
    Q_OBJECT
  public:
    explicit ViewManager(QObject* parent = nullptr);
    QUuid createView();
    // Создает окно с заданным ракурсом (удобно для Quad-View)
    QUuid createView(Visualize::ViewType type = Visualize::ViewType::VTK_3D);
    QUuid createView(Visualize::CameraViewType cameraType,
                     vtkRenderWindow*          existingWindow =
                         nullptr); // TODO: временная мера (заменить vtkRenderWindow на нормальный интерфес окон)
    void  setMainView(const QUuid& viewId);

    Visualize::IView* getView(const QUuid& viewId);
    QUuid             getMainViewId() const;
    void              removeView(const QUuid& id);

    // аргумент принимает указатель на IView
    template <typename Function> void forEachView(Function&& action) {
        for (auto& [id, renderer] : m_views) {
            action(renderer.get());
        }
    }
  signals:
    void viewCreated(QUuid id);
    void viewRemoved(QUuid id);
    void viewUpdateRequested();

  private:
    std::map<QUuid, std::unique_ptr<Visualize::IView>> m_views;
    QUuid                                              m_mainViewId;
};
} // namespace QSpace::Core
