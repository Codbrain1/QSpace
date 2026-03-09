#include <QTemporaryDir>
#include <QtTest>
#include <memory>
#include <vtkPolyData.h>


#include "Common/Structures/CoreStructures.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/SessionManager/SessionManager.h"
#include "Structures/SessionStructures.h"


using namespace QSpace::Core;
using namespace QSpace::Session;

class TestSessionManager : public QObject {
    Q_OBJECT

  private slots:
    void init() {
        m_registry       = new ObjectRegistry(this);
        m_sessionManager = new SessionManager(m_registry, this);
    }

    void cleanup() {
        delete m_sessionManager;
        delete m_registry;
    }

    void testSaveAndLoadEmptyProject() {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QString filePath = dir.path() + "/empty_project.qsp";

        CurrentSession session;
        session.projectName     = "EmptyProject";
        session.projectFilePath = filePath;

        // 1. Сохранение
        bool saveResult = m_sessionManager->saveProject(session);
        QVERIFY(saveResult);
        QVERIFY(QFile::exists(filePath));

        // 2. Загрузка
        auto loadedState = m_sessionManager->loadProject(filePath);
        QVERIFY(loadedState.has_value());
        QCOMPARE(loadedState->projectName, QString("empty_project.qsp"));
        QVERIFY(loadedState->nodesStates.isEmpty());
    }

    void testSaveAndLoadWithNodes() {
        QTemporaryDir dir;
        QString       filePath = dir.path() + "/full_project.qsp";

        // 1. Подготовка данных в реестре
        auto dummyData           = vtkSmartPointer<vtkPolyData>::New();
        auto node                = std::make_shared<DataNode>(dummyData, "Node1");
        node->path               = "/data/file.vtk";
        node->settings.PointSize = 5.0f;
        m_registry->registerNode(node);

        CurrentSession session;
        session.projectName     = "FullProject";
        session.projectFilePath = filePath;

        // 2. Сохранение
        QVERIFY(m_sessionManager->saveProject(session));

        // 3. Загрузка и проверка содержимого
        auto state = m_sessionManager->loadProject(filePath);
        QVERIFY(state.has_value());
        QCOMPARE(state->nodesStates.size(), 1);

        const auto& ds = state->nodesStates[0];
        QCOMPARE(ds.label, node->label);
        QCOMPARE(ds.path, node->path);
        QCOMPARE(ds.settings.PointSize, 5.0f);
    }

    void testLoadNonExistentFile() {
        auto state = m_sessionManager->loadProject("/non/existent/path.qsp");
        QVERIFY(!state.has_value());
    }

    void testSaveFailsOnEmptyPath() {
        CurrentSession session;
        session.projectName     = "NoPathProject";
        session.projectFilePath = ""; // Пустой путь

        bool result = m_sessionManager->saveProject(session);
        QVERIFY(!result);
    }

  private:
    ObjectRegistry* m_registry;
    SessionManager* m_sessionManager;
};

QTEST_MAIN(TestSessionManager)
#include "TestSessionManager.moc"