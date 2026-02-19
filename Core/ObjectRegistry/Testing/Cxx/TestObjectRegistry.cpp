#include <QSignalSpy>
#include <QtTest>
#include <memory>

// VTK включает для создания тестовых данных
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

// Твои заголовки
#include "Common/Structures/CoreStructures.h" // Путь к файлу с DataNode
#include "Core/ObjectRegistry/ObjectRegistry.h"

using namespace QSpace::Core;

class TestObjectRegistry : public QObject {
    Q_OBJECT

  private slots:
    // Вызывается перед каждым тестом: создаем чистый реестр
    void init() {
        m_registry = std::make_unique<ObjectRegistry>();
    }

    // Тест 1: Добавление ноды и проверка сигналов
    void testAddition() {
        QSignalSpy spy(m_registry.get(), &ObjectRegistry::nodeAdded);

        // Создаем минимальный объект VTK, чтобы DataNode не был "пустым"
        auto polyData = vtkSmartPointer<vtkPolyData>::New();
        auto points   = vtkSmartPointer<vtkPoints>::New();
        points->InsertNextPoint(0, 0, 0);
        polyData->SetPoints(points);

        // Теперь вызываем конструктор правильно (3 аргумента, 1 по умолчанию)
        auto node = std::make_shared<DataNode>(polyData, "Test Particle System", QSpace::Visualize::EntityType::Stars);

        m_registry->registerNode(node);

        // Проверки
        QCOMPARE(m_registry->getAllNodes().size(), 1);
        QCOMPARE(spy.count(), 1);

        // Проверяем, что в сигнале пришла именно наша нода
        auto signaledNode = spy.at(0).at(0).value<std::shared_ptr<DataNode>>();
        QVERIFY(signaledNode != nullptr);
        QCOMPARE(signaledNode->label, QString("Test Particle System"));
        QCOMPARE(signaledNode->stats.pointCount, 1); // Проверка логики внутри DataNode
    }

    // Тест 2: Поиск по UUID
    void testGetNode() {
        auto  node = std::make_shared<DataNode>(nullptr, "SearchMe");
        QUuid id   = node->id;

        m_registry->registerNode(node);

        auto found = m_registry->getNode(id);
        QVERIFY(found != nullptr);
        QCOMPARE(found->id, id);
        QCOMPARE(found->label, QString("SearchMe"));
    }

    // Тест 3: Удаление
    void testRemoval() {
        auto  node = std::make_shared<DataNode>(nullptr, "DeleteMe");
        QUuid id   = node->id;
        m_registry->registerNode(node);

        QSignalSpy spy(m_registry.get(), &ObjectRegistry::objectRemoved);

        m_registry->removeObject(id);

        QCOMPARE(m_registry->getAllNodes().size(), 0);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).value<QUuid>(), id);
    }

    // Тест 4: Бенчмарк на добавление (полезно для реестра)
    void testPerformance() {
        QBENCHMARK {
            for (int i = 0; i < 100; ++i) {
                auto node = std::make_shared<DataNode>(nullptr, QString("Node %1").arg(i));
                m_registry->registerNode(node);
            }
        }
    }

  private:
    std::unique_ptr<ObjectRegistry> m_registry;
};

// Не забудь зарегистрировать shared_ptr в метасистеме,
// если это еще не сделано в основном коде, иначе QSignalSpy его не вытащит.
Q_DECLARE_METATYPE(std::shared_ptr<QSpace::Core::DataNode>)

QTEST_MAIN(TestObjectRegistry)
#include "TestObjectRegistry.moc"