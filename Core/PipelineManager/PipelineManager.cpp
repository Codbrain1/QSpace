#include "Core/PipelineManager/PipelineManager.h"
#include "Common/Logger/Logger.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Renderer/Renderer.h"
#include <memory>
#include <qobject.h>
#include <vtkActor.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
namespace QSpace::Core {
PipelineManager::PipelineManager(ObjectRegistry* registry,
                                 ViewManager*    viewManager,
                                 LayerManager*   layerManager,
                                 QObject*        parent)
    : QObject(parent), m_viewManager(viewManager), m_layerManager(layerManager), m_registry(registry) {
    connect(m_registry, &ObjectRegistry::nodeAdded, this, &PipelineManager::onNodeAdded);
    connect(m_registry, &ObjectRegistry::objectRemoved, this, &PipelineManager::onObjectRemoved);
    connect(m_viewManager, &ViewManager::viewCreated, this, &PipelineManager::onViewCreated);
}
void PipelineManager::onNodeAdded(std::shared_ptr<DataNode> node) {
    qCInfo(LogCore) << "PipelineManager::onNodeAdded - Node added:" << node->label;

    // пробегаем по всем окнам
    m_viewManager->forEachView([this, node](Visualize::Renderer* renderer) {
        qCInfo(LogCore) << "PipelineManager::onNodeAdded - Creating layer in renderer";

        // создаем слой
        m_layerManager->createLayer(node, renderer);
        // Вызываем resetCamera() чтобы камера была настроена на новые данные
        try {
            renderer->resetCamera();
            qCInfo(LogCore) << "PipelineManager::onNodeAdded - Camera reset";
        } catch (...) {
            qCWarning(LogCore) << "PipelineManager::onNodeAdded - resetCamera() failed (no GL context?)";
        }
        renderer->render();
    });
}
void PipelineManager::onObjectRemoved(const QUuid& id) {
    // Удаляем объект из ВСЕХ окон
    qCInfo(LogCore) << "PipelineManager::onObjectRemoved - Object removed:" << id;
    m_layerManager->removeLayer(id);
    m_viewManager->forEachView([](Visualize::Renderer* r) { r->render(); });
}
void PipelineManager::onViewCreated(const QUuid& viewId) {
    // Открылось НОВОЕ окно. Оно пустое.
    qCInfo(LogCore) << "PipelineManager::onViewCreated - View created:" << viewId;
    Visualize::Renderer* newRenderer = m_viewManager->getView(viewId);
    if (!newRenderer)
        return;

    // Проходим по всем уже загруженным данным в Registry
    // (В ObjectRegistry нужен метод getAllNodes() или итератор)
    auto allNodes = m_registry->getAllNodes(); // Предполагаем наличие метода

    for (auto node : allNodes) {
        m_layerManager->createLayer(node, newRenderer);
    }

    newRenderer->resetCamera();
}
} // namespace QSpace::Core

// // neuro
// //  В PipelineManager::onNodeAdded
// void PipelineManager::onNodeAdded(std::shared_ptr<DataNode> node) {
//     m_viewManager->forEachView([this, node](Visualize::Renderer* renderer) {
//         // Создаем слой
//         auto layer = Visualize::LayerFactory::createLayer(node);
//         if (auto particleLayer = std::dynamic_pointer_cast<Visualize::ParticleLayer>(layer)) {
//             // Добавляем проп данных
//             renderer->addProp(particleLayer->getVtkProp());
//             // Добавляем легенду (колорбар)
//             renderer->addScalarBar(particleLayer->getScalarBar());
//         }
//         // ... сохраняем layer в LayerManager ...
//     });
// }
