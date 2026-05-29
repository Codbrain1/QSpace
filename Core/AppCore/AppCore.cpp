#include "AppCore.h"
#include "Common/Enums/IOEnums.h"
#include "Common/Interfaces/IView.h"
#include "Common/Logger/Logger.h"
#include "Core/DataManager/DataManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
// #include "Core/PipelineManager/PipelineManager.h"
#include "Core/TaskManager/TaskManager.h"
#include "Interfaces/IOFactory.h"
#include "Interfaces/IView.h"
#include "Visualize/VtkView.h"
#include "Enums/RenderEnums.h"
#include <memory>

#include "Core/DataManager/DataManager.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/SessionManager/SessionManager.h"
#include "Core/TaskManager/TaskManager.h"
#include "Core/ViewManager/ViewManager.h"

#include "DataController.h"
#include "ProjectController.h"
#include "VideoController.h"
#include "ViewController.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <qfileinfo.h>
#include <qloggingcategory.h>
#include <quuid.h>

namespace QSpace::Core {
AppCore::AppCore(QObject* parent) : QObject(parent) {
    // 1. Инициализация менеджеров (Базовый слой)
    m_taskManager    = std::make_unique<TaskManager>();
    m_objectRegistry = std::make_unique<ObjectRegistry>();
    m_dataManager    = std::make_unique<DataManager>(m_taskManager.get());
    m_viewManager    = std::make_unique<ViewManager>();
    m_layerManager   = std::make_unique<LayerManager>();
    m_sessionManager =
        std::make_unique<SessionManager>(m_objectRegistry.get(), m_layerManager.get(), this);

    // 2. Инициализация контроллеров (Внедрение зависимостей)
    m_viewController = std::make_unique<Controllers::ViewController>(m_viewManager.get(),
                                                                     m_layerManager.get(),
                                                                     m_objectRegistry.get(),
                                                                     this);

    m_dataController = std::make_unique<Controllers::DataController>(m_dataManager.get(),
                                                                     m_objectRegistry.get(),
                                                                     m_layerManager.get(),
                                                                     m_viewManager.get(),
                                                                     this);

    m_projectController = std::make_unique<Controllers::ProjectController>(m_sessionManager.get(),
                                                                           m_objectRegistry.get(),
                                                                           m_dataManager.get(),
                                                                           m_dataController.get(),
                                                                           this);

    m_videoController = std::make_unique<Controllers::VideoController>(m_dataManager.get(),
                                                                       m_viewManager.get(),
                                                                       m_layerManager.get(),
                                                                       this);
}

void AppCore::initialize() {
    // настройка PipelineManager для реагирования на изменения в ObjectRegistry и ViewManager
    // УДАЛЕНО, В БУДУЩЕМ МОЖЕТ ПОНАДОБИТСЯ, НО СЕЙЧАС ДУБЛИРУЕТ AppCore
    // connect(m_objectRegistry.get(),
    //         &ObjectRegistry::nodeAdded,
    //         m_pipelineManager.get(),
    //         &PipelineManager::onNodeAdded);
    // connect(m_objectRegistry.get(),
    //         &ObjectRegistry::objectRemoved,
    //         m_pipelineManager.get(),
    //         &PipelineManager::onObjectRemoved);
    // connect(m_viewManager.get(),
    //         &ViewManager::viewCreated,
    //         m_pipelineManager.get(),
    //         &PipelineManager::onViewCreated);
}

Controllers::ViewController* AppCore::viewController() const {
    return m_viewController.get();
}

Controllers::DataController* AppCore::dataController() const {
    return m_dataController.get();
}

Controllers::ProjectController* AppCore::projectController() const {
    return m_projectController.get();
}

Controllers::VideoController* AppCore::videoController() const {
    return m_videoController.get();
}

} // namespace QSpace::Core