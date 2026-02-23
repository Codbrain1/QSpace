#include "ViewManager.h"
#include "Enums/RenderEnums.h"
#include <qobject.h>
#include <quuid.h>
namespace QSpace::Core {
ViewManager::ViewManager(QObject* parent) : QObject(parent) {
}
QUuid ViewManager::createView() {
    return createView(Visualize::CameraViewType::Iso);
}
QUuid ViewManager::createView(Visualize::CameraViewType type) {
    auto renderer = std::make_unique<Visualize::Renderer>();
    renderer->setCameraView(type);
    renderer->resetCamera();

    QUuid id = QUuid::createUuid();
    m_views.emplace(id, std::move(renderer));
    if (m_mainViewId.isNull()) {
        m_mainViewId = id;
    }
    emit viewCreated(id);
    return id;
}

Visualize::Renderer* ViewManager::getView(const QUuid& viewId) {
    if (m_views.contains(viewId)) {
        return m_views[viewId].get();
    }
    return nullptr;
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
