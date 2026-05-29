#pragma once
#include "Common/Enums/IOEnums.h"
#include "Core/DataManager/DataManager.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
// #include "Core/PipelineManager/PipelineManager.h"
#include "Core/SessionManager/SessionManager.h"
#include "Core/TaskManager/TaskManager.h"
#include "Core/VideoExportManager/VideoExportManager.h"
#include "Core/ViewManager/ViewManager.h"
#include "Interfaces/IView.h"
#include <QObject>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <quuid.h>
#include <vtkRenderWindow.h>
#include "Structures/CoreStructures.h"
#include "Structures/IOStructures.h"
#include "Structures/RenderStructures.h"
#include "Structures/SessionStructures.h"
#include <functional>
#include <memory>
#include <mutex>

// Forward declarations для ускорения компиляции
namespace QSpace::Core {
class TaskManager;
class DataManager;
class ObjectRegistry;
class ViewManager;
class LayerManager;
class SessionManager;
class VideoExportManager;
} // namespace QSpace::Core

namespace QSpace::Models {
class DataTreeModel;
} // namespace QSpace::Models

// Forward declarations Контроллеров
namespace QSpace::Controllers {
class ViewController;
class DataController;
class ProjectController;
class VideoController;
} // namespace QSpace::Controllers

namespace QSpace::Core {
class AppCore : public QObject {
    Q_OBJECT
  public:
    explicit AppCore(QObject* parent = nullptr);
    ~AppCore();
    // ---------------------------------------------------------
    // @SECTION: инициализация
    // ---------------------------------------------------------
    /**
     * @brief initialize --- соединяет слоты AppCore с сигналами DataManager
     * @details
     * обрабатывает события начала чтения ioStarted(const QUuid& taskId, const QString& description,
     * int total); событие готовности файла fileReady(const QUuid& taskId, IO::ReadResult result,
     * IO::ImportRole role); событие завершения процесса чтения ioFinished(const QUuid& taskId, bool
     * succes);
     */
    void initialize();

    // ---------------------------------------------------------
    // @SECTION: модели данных
    // ---------------------------------------------------------
    Models::DataTreeModel* dataTreeModel() const;

    // ---------------------------------------------------------
    // @SECTION: контроллеры
    // ---------------------------------------------------------

    Controllers::ViewController*    viewController() const;
    Controllers::DataController*    dataController() const;
    Controllers::ProjectController* projectController() const;
    Controllers::VideoController*   videoController() const;

  private:
    // контроллеры управляющие разными областями программы
    std::unique_ptr<Controllers::ViewController>    m_viewController;
    std::unique_ptr<Controllers::DataController>    m_dataController;
    std::unique_ptr<Controllers::ProjectController> m_projectController;
    std::unique_ptr<Controllers::VideoController>   m_videoController;

    std::unique_ptr<TaskManager>        m_taskManager;
    std::shared_ptr<VideoExportManager> m_videoExportManager;
    std::unique_ptr<ObjectRegistry>     m_objectRegistry;
    std::unique_ptr<DataManager>        m_dataManager;
    std::unique_ptr<ViewManager>        m_viewManager;
    std::unique_ptr<LayerManager>       m_layerManager;
    std::unique_ptr<SessionManager>     m_sessionManager;
    QSpace::Session::CurrentSession     m_session_state;
};
} // namespace QSpace::Core