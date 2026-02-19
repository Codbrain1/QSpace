#include <QSignalSpy>
#include <QtTest>
#include <memory>


#include "Common/Structures/CoreStructures.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/PipelineManager/PipelineManager.h"
#include "Core/ViewManager/ViewManager.h"


using namespace QSpace::Core;

class TestPipelineManager : public QObject {
    Q_OBJECT

  private slots:
    void init() {
        m_registry     = new ObjectRegistry(this);
        m_viewManager  = new ViewManager(this);
        m_layerManager = new LayerManager(this);
        m_pipeline     = new PipelineManager(m_registry, m_viewManager, m_layerManager, this);
    }

    void cleanup() {
        delete m_pipeline;
        delete m_registry;
        delete m_viewManager;
        delete m_layerManager;
    }

    void testNodeAddition() {
        // 1. Создаем окно (это создаст Renderer)
        QUuid vId = m_viewManager->createView();

        // 2. Следим за LayerManager
        QSignalSpy spy(m_layerManager, &LayerManager::layerCreated);

        // 3. Создаем и регистрируем ноду
        // ВАЖНО: PipelineManager вызовет renderer->render()
        auto node = std::make_shared<DataNode>(nullptr, "TestNode");

        // Чтобы не упасть на Render(), можно добавить проверку в самом Renderer
        // Но сейчас попробуем просто вызвать регистрацию
        m_registry->registerNode(node);

        QCOMPARE(spy.count(), 1);
    }

    void testViewCreation() {
        // 1. Добавляем ноду заранее
        auto node = std::make_shared<DataNode>(nullptr, "EarlyNode");
        m_registry->registerNode(node);

        QSignalSpy spy(m_layerManager, &LayerManager::layerCreated);

        // 2. Создаем окно. PipelineManager должен подхватить ноду
        m_viewManager->createView();

        QCOMPARE(spy.count(), 1);
    }

  private:
    ObjectRegistry*  m_registry;
    ViewManager*     m_viewManager;
    LayerManager*    m_layerManager;
    PipelineManager* m_pipeline;
};

// Регистрируем типы для QVariant
Q_DECLARE_METATYPE(std::shared_ptr<QSpace::Core::DataNode>)

QTEST_MAIN(TestPipelineManager)
#include "TestPipelineManager.moc"