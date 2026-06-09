#include "ViewController.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/ViewManager/ViewManager.h"

namespace QSpace::Core::Controllers {

ViewController::ViewController(Core::ViewManager*    viewManager,
                               Core::LayerManager*   layerManager,
                               Core::ObjectRegistry* objectRegistry,
                               QObject*              parent)
    : QObject(parent),
      m_viewManager(viewManager),
      m_layerManager(layerManager),
      m_objectRegistry(objectRegistry) {
}

void ViewController::initialize() {
    // Пробрасываем сигналы менеджера наружу (чтобы UI слушал контроллер)
    connect(m_viewManager, &Core::ViewManager::viewCreated, this, &ViewController::viewCreated);
    connect(m_viewManager, &Core::ViewManager::viewRemoved, this, &ViewController::viewRemoved);
    connect(m_viewManager,
            &Core::ViewManager::viewUpdateRequested,
            this,
            &ViewController::sceneUpdateRequested);
    connect(m_objectRegistry, &Core::ObjectRegistry::objectRemoved, this, [this](const QUuid& id) {
        m_layerManager->removeLayer(id);
        emit sceneUpdateRequested();
    });
}

QUuid ViewController::createView(Visualize::ViewType type) {
    QUuid viewId = m_viewManager->createView(type);
    if (viewId.isNull())
        return QUuid();

    auto newView = m_viewManager->getView(viewId);
    if (m_layerManager && newView) {
        // Передаем созданное окно и ВСЕ ноды из реестра,
        // чтобы LayerManager создал для них слои в этом окне
        m_layerManager->populateNewView(newView, m_objectRegistry->getAllNodes());
    }
    return viewId;
}

void ViewController::removeView(const QUuid& viewId) {
    m_viewManager->removeView(viewId);
    emit viewRemoved(viewId);
}

Visualize::IView* ViewController::getView(const QUuid& viewId) {
    // Реализация (например, поиск в менеджере окон)
    return m_viewManager->getView(viewId).get();
}

void ViewController::resetCameraInAllViews() {
    m_viewManager->forEachView([](std::shared_ptr<Visualize::IView> r) { r->resetCamera(); });
}

// void AppCore::setGlobalExposureAllViews(double exposure) {
//     m_viewManager->forEachView([exposure](Visualize::IView* r) { r->setGlobalExposure(exposure);
//     });
// }

// void AppCore::setGlobalExposureView(const QUuid& viewId, double exposure) {
//     auto renderer = m_viewManager->getView(viewId);
//     if (renderer) {
//         renderer->setGlobalExposure(exposure);
//     }
// }
// void AppCore::setCameraViewInAllViews(Visualize::CameraViewType viewType) {
//     m_viewManager->forEachView([viewType](Visualize::IView* r) { r->setCameraView(viewType); });
// }
void ViewController::setBackgroundColorInAllViews(float r, float g, float b) {
    m_viewManager->forEachView(
        [r, g, b](std::shared_ptr<Visualize::IView> rw) { rw->setBackgroundColor(r, g, b); });
}

void ViewController::setAxesVisibleInAllViews(bool visible) {
    m_viewManager->forEachView(
        [visible](std::shared_ptr<Visualize::IView> rw) { rw->setAxesVisible(visible); });
}

void ViewController::setGridVisibleInAllViews(bool visible) {
    m_viewManager->forEachView(
        [visible](std::shared_ptr<Visualize::IView> rw) { rw->setGridVisible(visible); });
}

void ViewController::resetCameraInView(const QUuid& viewId) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->resetCamera();
    }
}

// void AppCore::setCameraViewInView(const QUuid& viewId, Visualize::CameraViewType viewType) {
//     auto renderer = m_viewManager->getView(viewId);
//     if (renderer) {
//         renderer->setCameraView(viewType);
//     }
// }

void ViewController::setBackgroundColorInView(const QUuid& viewId, float r, float g, float b) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->setBackgroundColor(r, g, b);
    }
}

void ViewController::setAxesVisibleInView(const QUuid& viewId, bool visible) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->setAxesVisible(visible);
    }
}

void ViewController::setGridVisibleInView(const QUuid& viewId, bool visible) {
    auto renderer = m_viewManager->getView(viewId);
    if (renderer) {
        renderer->setGridVisible(visible);
    }
}
} // namespace QSpace::Core::Controllers