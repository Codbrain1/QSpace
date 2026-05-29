#pragma once
#include "Common/Enums/IOEnums.h"
#include <QMap>
#include <QObject>
#include <QStringList>
#include <quuid.h>
#include "Enums/CoreEnums.h"
#include "Structures/SessionStructures.h"

// Forward declarations в пространствах имен проекта
namespace QSpace::Core {
class DataManager;
class ObjectRegistry;
class LayerManager;
class ViewManager;
class DataNode;
class DataContainer;
} // namespace QSpace::Core

namespace QSpace::Controllers {

// ---------------------------------------------------------
// @SECTION: загрузка данных
// ---------------------------------------------------------
class DataController : public QObject {
    Q_OBJECT
  public:
    explicit DataController(Core::DataManager*    dataManager,
                            Core::ObjectRegistry* objectRegistry,
                            Core::LayerManager*   layerManager,
                            Core::ViewManager*    viewManager,
                            QObject*              parent = nullptr);

    void initialize();
    /**
     * @brief importFiles --- Импортирует файлы с данными по указанным путям.
     * @param paths Список путей к файлам с данными
     */
    void importFiles(const QStringList&            paths,
                     Core::ModelingProgrammVersion version = Core::ModelingProgrammVersion::V2);
    /**
     * @brief importExperiment --- Импортирует данные нового эксперимента из указанной папки.
     * @param experimentName Имя контейнера эксперимента (например, имя папки)
     * @param filePaths Список абсолютных путей к бинарным файлам симуляции
     */
    void
    importExperiment(const QString&                experimentPath,
                     Core::ModelingProgrammVersion version); // TODO: версионирование временная мера

    void removeNodeObject(const QUuid& id);
    /**
     * @brief updateNodeSettings() --- обновляет данные записи в ObjectRegister
     * @param id --- уникальный идентификатор записи
     * @param modifer --- ссылка на функцию изменяющуюю данные записи, обязательно имеет
     * единственный парметор VisualSettings
     */
    void updateNodeSettings(const QUuid& id, std::function<void(Core::VisualSettings&)> modifier);

    // Метод, который вызовет ProjectController при открытии проекта
    void prepareNodesForRestoration(const QMap<QString, Session::DataNodeState>& restoringNodes);

    std::shared_ptr<QSpace::Core::DataNode> getNodeById(const QUuid& nodeId);
    QUuid                                   getNodePaletteId(const QUuid& nodeId);

    void createLayerForNode(const QUuid& nodeId);
  signals:
    void nodeAdded(std::shared_ptr<QSpace::Core::DataNode> node);
    void nodeContaineAdded(std::shared_ptr<QSpace::Core::DataContainer> container);
    void dataObjectRemoved(const QUuid& id);
    void sceneUpdateRequested();
    void markSessionDirty(); // Сигнал для ProjectController

  private slots:
    /**
     * @brief добавляет считанный файл в ObjectRegister
     */
    void onFileReady(const QUuid& taskId, IO::ReadResult result, IO::ImportRole role);

  private:
    Core::DataManager*    m_dataManager;
    Core::ObjectRegistry* m_objectRegistry;
    Core::LayerManager*   m_layerManager;
    Core::ViewManager*    m_viewManager;
    QMap<QUuid, QUuid>    m_taskToExperiment;

    QMap<QUuid, int>                      m_activeTasks;
    QMap<QString, Session::DataNodeState> m_restoringNodes; // Переехало сюда!
    bool                                  m_autoGrouping = true;

    QString                              extractGroupName(const QString& filename);
    std::shared_ptr<Core::DataContainer> findOrCreateContainer(const QString& groupName);
};
} // namespace QSpace::Controllers