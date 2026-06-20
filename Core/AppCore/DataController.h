#pragma once
#include "Common/Enums/IOEnums.h"
#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <quuid.h>
#include "Enums/CoreEnums.h"
#include "Physics/Math/MetaDataCalculating.h"
#include "Structures/SessionStructures.h"

// Forward declarations в пространствах имен проекта
namespace QSpace::Core {
class DataManager;
class ObjectRegistry;
class LayerManager;
class ViewManager;
class DataNode;
class Snapshot;
} // namespace QSpace::Core

namespace QSpace::Core::Controllers {

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

    // TODO: версионирование для чтения данных заменить на использование заранее подготовленных схем
    // чтения (добавить пользователю возможность самостоятельно создавать схемы для чтения файлов)

    //  --- Импорт и загрузка данных ---

    void importFiles(const QStringList&            paths,
                     Core::ModelingProgrammVersion version = Core::ModelingProgrammVersion::V2,
                     const QUuid&                  targetExperimentId = QUuid());

    void importExperiment(
        const QString&                experimentPath,
        Core::ModelingProgrammVersion version = Core::ModelingProgrammVersion::V2); // TODO

    void
    importExperiment(const QStringList&            filePaths,
                     const QString&                experimentName,
                     Core::ModelingProgrammVersion version = Core::ModelingProgrammVersion::V2);

    std::optional<QUuid> getExperimentIdByNodePath(const QString& nodePath) const;
    std::optional<QUuid> getNodeIdByFilePath(const QString& filePath) const;

    // --- Управление узлами данных (Nodes) ---
    void                                    createLayerForNode(const QUuid& nodeId);
    std::shared_ptr<QSpace::Core::DataNode> getNodeById(const QUuid& nodeId);
    QUuid                                   getNodePaletteId(const QUuid& nodeId);

    // Метод, который вызовет ProjectController при открытии проекта
    void prepareNodesForRestoration(const QMap<QString, Session::DataNodeState>& restoringNodes);

    QList<std::shared_ptr<QSpace::Core::Experiment>> getExperiments() const;

    double getSnapshotTimeByIndex(int index) {
        if (index > 0 && index < m_timeSliderSnapshots.size())
            return m_timeSliderSnapshots[index].timestamp;
        else
            return 0;
    };

  signals:
    void sceneUpdateRequested();
    void markSessionDirty(); // Сигнал для ProjectController
    void snapshotsListSizeChanged(const int size);

  public slots:
    void removeNodeObject(const QUuid& id);
    /**
     * @brief updateNodeSettings() --- обновляет данные записи в ObjectRegister
     * @param id --- уникальный идентификатор записи
     * @param modifer --- ссылка на функцию изменяющуюю данные записи, обязательно имеет
     * единственный парметор VisualSettings
     */
    void updateNodeSettings(const QUuid& id, std::function<void(Core::VisualSettings&)> modifier);
    // при изменении слайдера
    void handleTimeSliderValueChanged(int index, bool isPreview = false);
    // при выборе ноды в плоском режиме
    void onNodeSelectionActivated(const QUuid& nodeId);
    void handleTargetExperimentVisualizeChanged(const QUuid& experimentId);


  private slots:
    void handleFileReady(const QUuid& taskId, IO::ReadResult result);
    void onRequestDataLoad(const QUuid& nodeId);

  private:
    struct TaskInfo {
        QUuid targetNodeId;   // Будет пустым для первичного импорта пакета
        int   totalFiles = 1; // Количество файлов в задаче
    };

    struct SnapshotToTimestamp {
        QUuid  targetSnapshot;
        double timestamp;

        bool operator<(const SnapshotToTimestamp& other) const {
            return timestamp < other.timestamp;
        }
    };

    Core::DataManager* m_dataManager;

    Core::ObjectRegistry* m_objectRegistry;
    Core::LayerManager*   m_layerManager;
    Core::ViewManager*    m_viewManager;
    QMap<QUuid, QUuid>    m_taskToExperiment;

    QMap<QUuid, TaskInfo>                 m_activeTasks;
    QSet<QUuid>                           m_loadingNodes;
    QMap<QString, Session::DataNodeState> m_restoringNodes;

    bool m_autoGrouping = true;

    // ------- Обработка переключения кадров -------
    QUuid                            m_currentVisualizeExperimentId;
    QUuid                            m_currentActiveSnapshotId;
    std::vector<SnapshotToTimestamp> m_timeSliderSnapshots;
    // ---------------------------------------------

    QString                         extractGroupName(const QString& filename);
    std::shared_ptr<Core::Snapshot> findOrCreateSnapshot(const QString& groupName);
    void activateSnapshotInternal(const QUuid& snapshotId, bool isPreview);
};
} // namespace QSpace::Core::Controllers