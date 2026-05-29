#include "ProjectController.h"
#include "Common/Logger/Logger.h"
#include "Core/DataManager/DataManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/SessionManager/SessionManager.h"
#include <QFileInfo>
#include "DataController.h"

namespace QSpace::Controllers {

ProjectController::ProjectController(Core::SessionManager* sessionManager,
                                     Core::ObjectRegistry* objectRegistry,
                                     Core::DataManager*    dataManager,
                                     DataController*       dataController,
                                     QObject*              parent)
    : QObject(parent),
      m_sessionManager(sessionManager),
      m_objectRegistry(objectRegistry),
      m_dataManager(dataManager),
      m_dataController(dataController) {
}

void ProjectController::initialize() {
    // Слушаем DataController. Если там удалили или изменили ноду, помечаем сессию "грязной"
    connect(m_dataController, &Controllers::DataController::markSessionDirty, this, [this]() {
        m_sessionState.isDirty = true;
        emit sessionStateChanged(m_sessionState);
    });
}

void ProjectController::createNewProject(const QString& projectName) {
    m_objectRegistry->clear();
    m_sessionState.projectName = projectName;
    m_sessionState.projectFilePath.clear();
    m_sessionState.isDirty = false;
    emit sessionStateChanged(m_sessionState);
}

void ProjectController::saveCurrentProject() {
    if (m_sessionState.projectFilePath.isEmpty()) {
        emit requestSavePathFromUI();
        return;
    }
    saveCurrentProjectAs(m_sessionState.projectFilePath);
}

void ProjectController::saveCurrentProjectAs(const QString& projectPath) {
    if (projectPath.isEmpty())
        return;

    m_sessionState.projectFilePath = projectPath;
    if (m_sessionState.projectName.isEmpty()) {
        m_sessionState.projectName = QFileInfo(projectPath).baseName();
    }

    bool ok = m_sessionManager->saveProject(m_sessionState);
    if (ok) {
        m_sessionState.isDirty = false;
        emit sessionStateChanged(m_sessionState);
    } else {
        qCCritical(LogCore) << "Failed to save project to:" << projectPath;
    }
}

void ProjectController::openProject(const QString& projectPath) {
    auto newState = m_sessionManager->loadProject(projectPath);
    if (!newState.has_value()) {
        qCCritical(LogCore) << "The project has not been opened: " << projectPath;
        return;
    }

    m_sessionState.projectName     = newState->projectName;
    m_sessionState.projectFilePath = projectPath;
    m_sessionState.isDirty         = false;

    // Очищаем старые данные перед загрузкой новых
    m_objectRegistry->clear();

    // 1. Передаем "шпаргалку" восстановления в DataController
    QMap<QString, Session::DataNodeState> restoringMap;
    for (const auto& nodeState : newState->nodesStates) {
        restoringMap.insert(nodeState.path, nodeState);
    }
    m_dataController->prepareNodesForRestoration(restoringMap);

    // 2. Запускаем асинхронный импорт файлов проекта
    for (const auto& nodeState : newState->nodesStates) {
        m_dataManager->importDataAsync(nodeState.path, nodeState.scheme);
    }

    emit sessionStateChanged(m_sessionState);
}

void ProjectController::loadPalette(const QString& filePath) {
    auto colorMapOpt = m_sessionManager->loadPalette(filePath);
    if (colorMapOpt.has_value()) {
        emit paletteLoaded(colorMapOpt.value());
    } else {
        qCWarning(LogCore) << "Failed to load palette from:" << filePath;
    }
}

void ProjectController::savePalette(const Visualize::ColorMap& map, const QString& filePath) {
    if (!m_sessionManager->savePalette(map, filePath)) {
        qCWarning(LogCore) << "Failed to save palette to:" << filePath;
    }
}

} // namespace QSpace::Controllers