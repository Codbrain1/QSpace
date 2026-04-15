#include <QSignalSpy>
#include <QtTest>
#include <memory>

#include "Common/Structures/CoreStructures.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/PipelineManager/PipelineManager.h"
#include "Core/ViewManager/ViewManager.h"
#include "Enums/RenderEnums.h"
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
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
        QUuid      vId = m_viewManager->createView(QSpace::Visualize::CameraViewType::Iso);
        QSignalSpy spy(m_layerManager, &LayerManager::layerCreated);

        // СОЗДАЕМ МИНИМАЛЬНЫЕ ДАННЫЕ
        auto dummyData = vtkSmartPointer<vtkPolyData>::New();
        auto node      = std::make_shared<DataNode>(dummyData, "TestNode");

        m_registry->registerNode(node);

        QCOMPARE(spy.count(), 1);
    }

    void testViewCreation() {
        // ТО ЖЕ САМОЕ ЗДЕСЬ
        auto dummyData = vtkSmartPointer<vtkPolyData>::New();
        auto node      = std::make_shared<DataNode>(dummyData, "EarlyNode");

        m_registry->registerNode(node);

        QSignalSpy spy(m_layerManager, &LayerManager::layerCreated);
        m_viewManager->createView(QSpace::Visualize::CameraViewType::Iso);

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