#pragma once
#include "Common/Enums/IOEnums.h"
#include "Core/DataManager/DataManager.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/PipelineManager/PipelineManager.h"
#include "Core/SessionManager/SessionManager.h"
#include "Core/TaskManager/TaskManager.h"
#include "Core/VideoExportManager/VideoExportManager.h"
#include "Core/ViewManager/ViewManager.h"
#include "Structures/CoreStructures.h"
#include "Structures/IOStructures.h"
#include "Structures/RenderStructures.h"
#include "Structures/SessionStructures.h"
#include <QObject>
#include <functional>
#include <memory>
#include <mutex>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>
#include <vtkRenderWindow.h>

namespace QSpace::Core {
class AppCore : public QObject {
    Q_OBJECT
  public:
    explicit AppCore(QObject* parent = nullptr);
    ~AppCore() = default;
    /**
     * @brief initialize() --- соединяет слоты AppCore с сигналами DataManager
     * @details
     * обрабатывает события начала чтения ioStarted(const QUuid& taskId, const QString& description, int total);
     * событие готовности файла fileReady(const QUuid& taskId, IO::ReadResult result, IO::ImportRole role);
     * событие завершения процесса чтения ioFinished(const QUuid& taskId, bool succes);
     */
    void initialize();
    /**
     * @brief setGroupingEnabled(bool enabled) --- в разработке
     */
    void setGroupingEnabled(bool enabled);

    /**
     * @brief  startVideoExport() --- инициализирует VideoExportManager для создания видео анимации
     * @param baseNodeId --- уникальный идентификатор слоя настройки которого исползуются для анимации
     * @param files --- список путей к файлам для анимирования (все файла должны быть семантически совместимы)
     * @param outputPath --- путь к выходному файлу с видео
     * @param stride --- шаг для пропуска файлов
     */
    void startVideoExport(const QUuid&       baseNodeId,
                          const QStringList& files,
                          const QString&     outputPath,
                          int                stride = 1,
                          int                fps    = 30);
    void importFiles(const QStringList& paths);
    void removeLayer(const QString& layerName);
    /**
     * @brief отменяет рендеринг видео
     */
    void cancelVideoExport();
    void saveCurrentProject();
    void saveCurrentProjectAs(const QString& projectPath);
    void openProject(const QString& projectPath);

    QUuid createView(Visualize::ViewType type = Visualize::ViewType::VTK_3D, vtkRenderWindow* existingWindow = nullptr);
    void  removeView(const QUuid& viewId);

    void resetCameraInAllViews();
    void setCameraViewInAllViews(Visualize::CameraViewType viewType);
    void setBackgroundColorInAllViews(float r, float g, float b);
    void setAxesVisibleInAllViews(bool visible);
    void setGridVisibleInAllViews(bool visible);
    void setGlobalExposureAllViews(double exposure);

    void  resetCameraInView(const QUuid& viewId);
    void  setCameraViewInView(const QUuid& viewId, Visualize::CameraViewType viewType);
    void  setBackgroundColorInView(const QUuid& viewId, float r, float g, float b);
    void  setAxesVisibleInView(const QUuid& viewId, bool visible);
    void  setGridVisibleInView(const QUuid& viewId, bool visible);
    void  setGlobalExposureView(const QUuid& viewId, double exposure);
    QUuid getNodePaletteId(const QUuid& nodeId);
    std::shared_ptr<QSpace::Core::DataNode> getNodeById(const QUuid& nodeId);

  public slots:
    /**
     * @brief updateNodeSettings() --- обновляет данные записи в ObjectRegister
     * @param id --- уникальный идентификатор записи
     * @param modifer --- ссылка на функцию изменяющуюю данные записи, обязательно имеет единственный парметор
     * VisualSettings
     */
    void updateNodeSettings(const QUuid& id, std::function<void(VisualSettings&)> modifer);
    void removeObject(const QUuid& id);
    void savePalette(const Visualize::ColorMap& map, const QString& filePath);
    void loadPalette(const QString& filePath);
  signals:
    /**
     * @brief сообщает об обновлении процесса создания видео
     * @param  currentFrame --- номер читаемого файла
     * @param  totalFrames --- общее число файлов
     */
    void exportProgressUpdated(int currentFrame, int totalFrames);
    /**
     * @brief сигнал об окончании создания видео
     */
    void exportFinished(bool success);
    /**
     * @brief сигнал о запросе пути для сохранения проекта
     */
    void requestSavePathFromUI();
    /**
     * @brief уведомляет об изменении состояния сессии
     */
    void sessionStateChanged(QSpace::Session::CurrentSession session);
    void nodeAdded(std::shared_ptr<QSpace::Core::DataNode> node);
    void objectRemoved(const QUuid& id);
    void sceneUpdateRequested();
    void paletteLoaded(QSpace::Visualize::ColorMap colorMap);
    void viewCreated(QUuid viewId, Visualize::ViewType type);
    void viewRemoved(QUuid viewId);
  private slots:
    /**
     * @brief добавляет считанный файл в ObjectRegister
     */
    void onFileReady(const QUuid& taskId, IO::ReadResult result, IO::ImportRole role);

    void createNewProject(const QString& projectName);

  private:
    std::unique_ptr<TaskManager>        m_taskManager;
    std::shared_ptr<VideoExportManager> m_videoExportManager;
    std::unique_ptr<ObjectRegistry>     m_objectRegistry;
    std::unique_ptr<DataManager>        m_dataManager;
    std::unique_ptr<ViewManager>        m_viewManager;
    std::unique_ptr<PipelineManager>    m_pipelineManager;
    std::unique_ptr<LayerManager>       m_layerManager;
    std::unique_ptr<SessionManager>     m_sessionManager;
    QSpace::Session::CurrentSession     m_session_state;
    QMap<QUuid, int>                    m_activeTasks;
    // Очередь восстановления: Путь файла -> Сохраненное состояние ноды
    QMap<QString, QSpace::Session::DataNodeState> m_restoringNodes;
    bool                                          m_autoGrouping  = true;
    bool                                          m_isInitialized = false;
    QString                                       extractGroupName(const QString& filename);
    std::shared_ptr<DataContainer>                findOrCreateContainer(const QString& groupName);
};
} // namespace QSpace::Core