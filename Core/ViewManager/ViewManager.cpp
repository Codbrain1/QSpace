#include "ViewManager.h"
#include "Enums/RenderEnums.h"
#include "Interfaces/IView.h"
#include "Visualize/VtkView.h"
#include <memory>
#include <qobject.h>
#include <quuid.h>

namespace QSpace::Core {
ViewManager::ViewManager(QObject* parent) : QObject(parent) {
}
QUuid ViewManager::createView(Visualize::ViewType type) {
    std::unique_ptr<QSpace::Visualize::IView> newView = nullptr;
    switch (type) { // TODO: добавить фабрику окон
        case QSpace::Visualize::ViewType::VTK_3D: {
            auto vtkView = std::make_unique<Visualize::VtkView>();
            vtkView->setCameraView(Visualize::CameraViewType::Iso);
            vtkView->resetCamera();
            newView = std::move(vtkView);
            break;
        }
        case QSpace::Visualize::ViewType::Widget_2D: {
            break;
        }
    }
    if (!newView) {
        return QUuid();
    }
    connect(newView.get(), &Visualize::IView::updateRequested, this, &ViewManager::viewUpdateRequested);
    QUuid id = QUuid::createUuid();
    m_views.emplace(id, std::move(newView));
    if (m_mainViewId.isNull()) {
        m_mainViewId = id;
    }
    emit viewCreated(id);
    return id;
}
QUuid ViewManager::createView(Visualize::CameraViewType cameraType, vtkRenderWindow* existingWindow) {
    QUuid id = createView(Visualize::ViewType::VTK_3D);
    if (auto vtkView = dynamic_cast<Visualize::VtkView*>(getView(id))) {
        vtkView->setCameraView(cameraType);
        if (existingWindow) {
            vtkView->setRenderWindow(existingWindow);
        } else {
            // vtkView->initDefaultWindow(); // Создать свое, если не передали //TODO: исправить
        }
    }
    return id;
}

Visualize::IView* ViewManager::getView(const QUuid& viewId) {
    if (m_views.contains(viewId)) {
        return m_views[viewId].get();
    }
    return nullptr;
}
void ViewManager::setMainView(const QUuid& viewId) {
    if (m_views.contains(viewId)) {
        m_mainViewId = viewId;
    }
}
QUuid ViewManager::getMainViewId() const {
    return m_mainViewId;
}
void ViewManager::removeView(const QUuid& id) {
    if (m_views.contains(id)) {
        m_views.erase(id);
        if (m_mainViewId == id && !m_views.empty()) {
            m_mainViewId = m_views.begin()->first;
        }
        emit viewRemoved(id);
    }
}
} // namespace QSpace::Core
