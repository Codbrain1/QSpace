#include "Core/PipelineManager/PipelineManager.h"
#include "Common/Interfaces/IView.h"
#include "Common/Logger/Logger.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include <qobject.h>
#include <vtkActor.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <memory>

namespace QSpace::Core {
PipelineManager::PipelineManager(ObjectRegistry* registry,
                                 ViewManager*    viewManager,
                                 LayerManager*   layerManager,
                                 QObject*        parent)
    : QObject(parent),
      m_viewManager(viewManager),
      m_layerManager(layerManager),
      m_registry(registry) {
}

void PipelineManager::onNodeAdded(std::shared_ptr<DataNode> node) {
    qCInfo(LogCore) << "PipelineManager::onNodeAdded - Node added:" << node->label;

    // пробегаем по всем окнам
    m_viewManager->forEachView(
        [this, node](std::shared_ptr<QSpace::Visualize::IView> renderer) {
            qCInfo(LogCore) << "PipelineManager::onNodeAdded - Creating layer in renderer";

            // создаем слой
            m_layerManager->createLayer(node, renderer);
            // Вызываем resetCamera() чтобы камера была настроена на новые данные
            try {
                if (node->masterSettings->isVisible) {
                    renderer->resetCamera();
                    renderer->render();
                    qCInfo(LogCore) << "PipelineManager::onNodeAdded - Camera reset";
                }
            } catch (...) {
                qCWarning(LogCore)
                    << "PipelineManager::onNodeAdded - resetCamera() failed (no GL context?)";
            }
        });
}

void PipelineManager::onObjectRemoved(const QUuid& id) {
    // Удаляем объект из ВСЕХ окон
    qCInfo(LogCore) << "PipelineManager::onObjectRemoved - Object removed:" << id;
    m_layerManager->removeLayer(id);
    m_viewManager->forEachView([](std::shared_ptr<QSpace::Visualize::IView> r) { r->render(); });
}

void PipelineManager::onViewCreated(const QUuid& viewId) {
    // Открылось НОВОЕ окно. Оно пустое.
    qCInfo(LogCore) << "PipelineManager::onViewCreated - View created:" << viewId;
    std::shared_ptr<QSpace::Visualize::IView> newIView = m_viewManager->getView(viewId);
    if (!newIView)
        return;

    // Проходим по всем уже загруженным данным в Registry
    // (В ObjectRegistry нужен метод getAllNodes() или итератор)
    auto allNodes = m_registry->getAllNodes(); // Предполагаем наличие метода

    for (auto node : allNodes) {
        m_layerManager->createLayer(node, newIView);
    }

    newIView->resetCamera();
}
} // namespace QSpace::Core