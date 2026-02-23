#include "Core/AppCore/AppCore.h"
#include "AppCore.h"
#include "Common/Logger/Logger.h"
#include "Core/DataManager/DataManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/PipelineManager/PipelineManager.h"
#include "Core/TaskManager/TaskManager.h"
#include "Enums/RenderEnums.h"
#include "Interfaces/IOFactory.h"
#include "Structures/CoreStructures.h"
#include <functional>
#include <memory>
#include <qcontainerfwd.h>
#include <qfiledevice.h>
#include <qfileinfo.h>
#include <qloggingcategory.h>
#include <qobject.h>
#include <quuid.h>

namespace QSpace::Core {
AppCore::AppCore(QObject* parent) : QObject(parent) {
    m_taskManager    = std::make_unique<TaskManager>();
    m_objectRegistry = std::make_unique<ObjectRegistry>();
    m_dataManager    = std::make_unique<DataManager>(m_taskManager.get());
    m_viewManager    = std::make_unique<ViewManager>();
    m_layerManager   = std::make_unique<LayerManager>();
    m_pipelineManager =
        std::make_unique<PipelineManager>(m_objectRegistry.get(), m_viewManager.get(), m_layerManager.get());
}
void AppCore::initialize() {
    m_viewManager->createView();
    connect(m_dataManager.get(), &DataManager::fileReady, this, &AppCore::onFileReady);
}
void AppCore::onFileReady(IO::ReadResult result) {
    if (!result.isSuccess()) {
        qCWarning(LogCore) << "Error loading: " << result.errMessage;
        return;
    }
    QString               fileName = QFileInfo(result.path).fileName();
    Visualize::EntityType type     = IO::Utils::getEntityType(result.path);
    auto                  node     = std::make_shared<DataNode>(result.data, fileName, type);

    qCInfo(LogCore) << "AppCore::onFileReady - File loaded:" << fileName << "Points:" << node->stats.pointCount;

    if (m_autoGrouping) {
        QString groupName = extractGroupName(fileName);
        if (!groupName.isEmpty()) {
            auto container = findOrCreateContainer(groupName);
            container->addComponent(node);
            // Регистрируем ноду (она вызовет signal nodeAdded -> PipelineManager -> View)
            m_objectRegistry->registerNode(node);
            qInfo() << "Grouped" << fileName << "into" << groupName;
        } else {
            m_objectRegistry->registerNode(node);
        }
    } else {
        m_objectRegistry->registerNode(node);
    }

    m_viewManager->forEachView([](Visualize::Renderer* r) {
        r->resetCamera();
        r->getRenderWindow()->Render(); // Это теперь вызовет и RenderWindow->Render(), и updateRequested сигнал
    });

    qCInfo(LogCore) << "AppCore::onFileReady - File processing completed";
}
std::shared_ptr<DataContainer> AppCore::findOrCreateContainer(const QString& groupName) {
    auto existing = m_objectRegistry->findContainerByName(groupName);
    if (existing)
        return existing;

    auto newContainer = std::make_shared<DataContainer>(groupName);
    m_objectRegistry->registerContainer(newContainer);
    return newContainer;
}
QString AppCore::extractGroupName(const QString& filename) {
    // Ищем паттерны вида "snap_001", "step-500"
    static QRegularExpression regex(R"((snap(shot)?|step)[_\-]?\d+)", QRegularExpression::CaseInsensitiveOption);
    auto                      match = regex.match(filename);
    return match.hasMatch() ? match.captured(0) : QString();
}
void AppCore::updateNodeSettings(const QUuid& id, std::function<void(VisualSettings&)> modifer) {
    auto node = m_objectRegistry->getNode(id);
    if (node) {
        modifer(node->settings);
        m_layerManager->updateSettings(id);
    }
}
} // namespace QSpace::Core