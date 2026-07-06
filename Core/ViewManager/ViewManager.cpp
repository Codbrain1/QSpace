#include "ViewManager.h"
#include "Visualize/Views/ViewFactory.h"
#include <qobject.h>
#include <quuid.h>
#include <memory>

namespace QSpace::Core {
ViewManager::ViewManager(QObject* parent) : QObject(parent) {
}

QUuid ViewManager::createView(Visualize::ViewType type) {
    std::unique_ptr<QSpace::Visualize::IView> newView = nullptr;

    // 1. Генерируем ID заранее, так как он понадобится нам для настройки связей
    QUuid id = QUuid::createUuid();

    // CRITICAL испарвить на вызов фабричного метода
    // 2. Фабричная логика: создаем нужную реализацию
    switch (type) {
        case QSpace::Visualize::ViewType::VTK_3D: {
            auto vtkView = std::make_unique<Visualize::VtkView>();
            vtkView->setCameraView(Visualize::CameraViewType::Iso);
            vtkView->resetCamera();
            newView = std::move(vtkView);
            break;
        }
        case QSpace::Visualize::ViewType::Widget_2D: {
            // TODO: реализовать создание 2D окна
            // auto plotView = std::make_unique<Visualize::PlotView>();
            // newView = std::move(plotView);
            break;
        }
        default:
            // Можно добавить qWarning() << "Unknown ViewType requested";
            break;
    }

    // Проверка на случай неудачного создания или нереализованного типа (как Widget_2D сейчас)
    if (!newView) {
        return QUuid();
    }

    // 3. Настройка связей (ВАЖНО: Делаем это ДО перемещения unique_ptr в мапу)
    // Используем лямбду, чтобы захватить наш сгенерированный id и передать его дальше
    connect(newView.get(), &Visualize::IView::updateRequested, this, [this, id]() {
        emit viewUpdateRequested(id); // Адресное уведомление!
    });

    // Опционально: если у твоего интерфейса IView есть метод setId, самое время его вызвать
    // newView->setId(id);

    // 4. Перемещаем владение объектом в контейнер менеджера
    m_views.emplace(id, std::move(newView));

    // 5. Устанавливаем главное окно, если оно еще не задано
    if (m_mainViewId.isNull()) {
        m_mainViewId = id;
    }

    // 6. Уведомляем систему (AppCore -> MainWindow) о том, что окно создано
    emit viewCreated(id, type);

    return id;
}

std::shared_ptr<Visualize::IView> ViewManager::getView(const QUuid& viewId) {
    if (m_views.contains(viewId)) {
        return m_views[viewId];
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
        emit viewRemoved(id, m_views[id]->getViewType());
    }
}

void ViewManager::updateAllViews() {
    for (auto& [id, view] : m_views) {
        view->render();
    }
}

void ViewManager::updateView(const QUuid& id) {
    if (m_views.contains(id)) {
        m_views[id]->render();
    }
}

void ViewManager::renderAllViews() {
    for (auto& [id, view] : m_views) {
        view->renderForce();
    }
}

void ViewManager::renderView(const QUuid& id) {
    if (m_views.contains(id)) {
        m_views[id]->renderForce();
    }
}
} // namespace QSpace::Core
