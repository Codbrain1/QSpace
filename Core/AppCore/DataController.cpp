#include "DataController.h"

// Подключение внутренних менеджеров ядра
#include "Common/Structures/CoreStructures.h"
#include "Core/DataManager/DataManager.h"
#include "Core/LayerManager/LayerManager.h"
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Core/ViewManager/ViewManager.h"



// Системные и утилитарные заголовки
#include "Common/Logger/Logger.h"
#include "Interfaces/IOFactory.h"
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

namespace QSpace::Controllers {

DataController::DataController(Core::DataManager*    dataManager,
                               Core::ObjectRegistry* objectRegistry,
                               Core::LayerManager*   layerManager,
                               Core::ViewManager*    viewManager,
                               QObject*              parent)
    : QObject(parent),
      m_dataManager(dataManager),
      m_objectRegistry(objectRegistry),
      m_layerManager(layerManager),
      m_viewManager(viewManager) {
}

void DataController::initialize() {
    // 1. Подписываемся на события менеджера низкоуровневого ввода-вывода
    connect(m_dataManager, &Core::DataManager::fileReady, this, &DataController::onFileReady);

    connect(
        m_dataManager,
        &Core::DataManager::ioStarted,
        this,
        [this](const QUuid& taskId, const QString&, int total) { m_activeTasks[taskId] = total; });

    connect(m_dataManager,
            &Core::DataManager::ioFinished,
            this,
            [this](const QUuid& taskId, bool success) {
                if (success) {
                    // Пакетная загрузка завершена — запрашиваем финальную отрисовку сцены
                    emit sceneUpdateRequested();
                }
                m_activeTasks.remove(taskId);
            });

    // 2. Транслируем сигналы реестра объектов наружу для UI и Моделей
    connect(m_objectRegistry, &Core::ObjectRegistry::nodeAdded, this, &DataController::nodeAdded);
    connect(m_objectRegistry,
            &Core::ObjectRegistry::objectRemoved,
            this,
            &DataController::dataObjectRemoved);
}

void DataController::importFiles(const QStringList& paths, Core::ModelingProgrammVersion version) {
    if (paths.isEmpty())
        return;

    if (paths.size() == 1) {
        // Одиночный импорт файла
        IO::FileFormat        format     = IO::Utils::getFormat(paths[0]);
        Visualize::EntityType entityType = IO::Utils::getEntityType(QFileInfo(paths[0]).fileName());
        IO::ReadScheme        scheme;
        if (version == Core::ModelingProgrammVersion::V2_3) {
            scheme = IO::SchemeFactory::createSheme_v2_3(entityType, format);
        } else {
            scheme = IO::SchemeFactory::createSheme_v2(entityType, format);
        }
        m_dataManager->importDataAsync(paths[0], scheme);
    } else {
        // Пакетный асинхронный импорт файлов
        QList<IO::BatchTask> tasks;
        for (const auto& path : paths) {
            IO::FileFormat        format     = IO::Utils::getFormat(QFileInfo(path).fileName());
            Visualize::EntityType entityType = IO::Utils::getEntityType(QFileInfo(path).fileName());
            IO::ReadScheme        scheme;
            if (version == Core::ModelingProgrammVersion::V2_3) {
                scheme = IO::SchemeFactory::createSheme_v2_3(entityType, format);
            } else {
                scheme = IO::SchemeFactory::createSheme_v2(entityType, format);
            }

            IO::BatchTask task;
            task.path   = path;
            task.scheme = scheme;
            tasks.append(task);
        }
        m_dataManager->importBatchDataAsync(tasks);
    }
}

void DataController::importExperiment(const QString&                experimentPath,
                                      Core::ModelingProgrammVersion version) {
    if (experimentPath.isEmpty() || !m_dataManager || !m_objectRegistry) {
        return;
    }

    QDir dir(experimentPath);
    if (!dir.exists()) {
        return;
    }

    // 1. Получаем имя эксперимента из названия папки
    QString experimentName = dir.dirName();

    // 2. Создаем объект Эксперимента и регистрируем его в ОЗУ
    auto experiment = std::make_shared<Core::Experiment>(experimentName);
    experiment->id  = QUuid::createUuid();
    // experiment->name = experimentName; // Раскомментируйте, если в структуре Experiment есть поле
    // name
    m_objectRegistry->registerExperiment(experiment);

    // 3. Рекурсивно собираем все .bin файлы в папке
    QList<IO::BatchTask> tasks;
    QDirIterator         it(experimentPath,
                    QStringList() << "*.bin",
                    QDir::Files,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();

        // Формируем схему чтения на основе выбранной версии программы
        IO::ReadScheme scheme;
        // В зависимости от вашей структуры ReadScheme, настройте её здесь:
        // scheme.version = version;

        tasks.append({filePath, scheme});
    }

    if (tasks.isEmpty()) {
        qWarning() << "No .bin files found in" << experimentPath;
        return;
    }

    // 4. Запускаем асинхронное чтение и сохраняем связь с экспериментом
    QUuid taskId = m_dataManager->importBatchDataAsync(tasks, IO::ImportRole::ProjectData);

    if (!taskId.isNull()) {
        m_taskToExperiment.insert(taskId, experiment->id);
    }
}

void DataController::prepareNodesForRestoration(
    const QMap<QString, Session::DataNodeState>& restoringNodes) {
    m_restoringNodes = restoringNodes;
}

void DataController::removeNodeObject(const QUuid& id) {
    // Контроллер занимается только структурами данных.
    // ViewController поймает этот сигнал реактивно и зачистит VTK слои в окнах.
    m_objectRegistry->removeObject(id);
    emit markSessionDirty();
}

void DataController::updateNodeSettings(const QUuid&                               id,
                                        std::function<void(Core::VisualSettings&)> modifier) {
    auto node = m_objectRegistry->getNode(id);
    if (!node)
        return;

    // 1. Модифицируем мастер-настройки внутри структуры данных ноды
    modifier(*(node->masterSettings));

    // 2. Просим LayerManager раскатить изменения на все активные VTK-пайплайны
    m_layerManager->updateNodeMasterSettings(id);

    emit sceneUpdateRequested();
}

std::shared_ptr<Core::DataNode> DataController::getNodeById(const QUuid& nodeId) {
    return m_objectRegistry->getNode(nodeId);
}

QUuid DataController::getNodePaletteId(const QUuid& nodeId) {
    auto node = m_objectRegistry->getNode(nodeId);
    return node ? node->masterSettings->colorMapId : QUuid();
}

void DataController::onFileReady(const QUuid&           taskId,
                                 QSpace::IO::ReadResult result,
                                 QSpace::IO::ImportRole role) {
    if (role != IO::ImportRole::ProjectData)
        return;

    if (!result.isSuccess()) {
        qCWarning(LogCore) << "Error loading file:" << result.errMessage;
        m_restoringNodes.remove(result.path);
        return;
    }

    int                   totalInThisTask = m_activeTasks.value(taskId, 1);
    QString               fileName        = QFileInfo(result.path).fileName();
    Visualize::EntityType type            = IO::Utils::getEntityType(result.path);

    // 1. Создаем базовый узел данных (DataNode)
    auto node = std::make_shared<Core::DataNode>(result.data, fileName, result.timestamp, type);

    // 2. Восстановление состояния (при загрузке сохраненного проекта)
    if (m_restoringNodes.contains(result.path)) {
        auto restoredState = m_restoringNodes.take(result.path);

        node->id             = restoredState.id;
        node->label          = restoredState.label;
        node->masterSettings = restoredState.settings.clone();
        node->path           = restoredState.path;
        node->format         = restoredState.format;
        node->scheme         = restoredState.scheme;
        node->type           = restoredState.type;
    } else {
        // Обычная логика инициализации для новых файлов импорта
        node->masterSettings->isVisible = (totalInThisTask == 1);
        node->path                      = result.path;
        node->format                    = result.format;
        node->scheme                    = result.scheme;

        emit markSessionDirty();
    }

    qCInfo(LogCore) << "DataController::onFileReady - Успешно загружен:" << fileName
                    << "| Количество точек:" << node->stats.pointCount;

    // 3. Автоматическая группировка (Контейнеры / Снапшоты)
    bool isAddedToContainer = false;
    if (m_autoGrouping) {
        QString groupName = extractGroupName(fileName);
        if (!groupName.isEmpty()) {
            auto container = findOrCreateContainer(groupName);
            container->addComponent(node);
            isAddedToContainer = true;
            qInfo() << "Файл" << fileName << "сгруппирован в контейнер:" << groupName;
        }
    }

    // 4. Регистрация в ObjectRegistry
    // Проверяем, принадлежит ли текущая задача чтения какому-либо Эксперименту
    if (m_taskToExperiment.contains(taskId)) {
        QUuid experimentId = m_taskToExperiment.value(taskId);
        m_objectRegistry->registerNodeWithGrouping(node, experimentId);
    } else {
        // Если это одиночный импорт вне эксперимента
        m_objectRegistry->registerNode(node);
    }

    // 5. Визуализация (Создание слоев)
    // Если файл попал в контейнер, мы НЕ создаем для него слой сразу
    // (слои для контейнеров обычно создаются при раскрытии дерева или специальной командой).
    // Если же файл одиночный — сразу генерируем представления во всех окнах.
    if (!isAddedToContainer) {
        if (m_viewManager && m_layerManager) {
            m_viewManager->forEachView([&](std::shared_ptr<Visualize::IView> view) {
                m_layerManager->createLayer(node, view);
            });
        }
    }

    // 6. Оповещаем систему
    emit nodeAdded(node);
}

QString DataController::extractGroupName(const QString& filename) {
    // Регулярное выражение для группировки шагов симуляций (например: snap_001, step-500)
    static QRegularExpression regex(R"((snap(shot)?|step)[_\-]?\d+)",
                                    QRegularExpression::CaseInsensitiveOption);
    auto                      match = regex.match(filename);
    return match.hasMatch() ? match.captured(0) : QString();
}

std::shared_ptr<Core::DataContainer>
DataController::findOrCreateContainer(const QString& groupName) {
    auto existing = m_objectRegistry->findContainerByName(groupName);
    if (existing)
        return existing;

    auto newContainer = std::make_shared<Core::DataContainer>(groupName);
    m_objectRegistry->registerContainer(newContainer);
    return newContainer;
}

void DataController::createLayerForNode(const QUuid& nodeId) {
    // 1. Получаем саму ноду данных из реестра или через внутренний метод
    auto node = getNodeById(nodeId);
    if (!node)
        return;

    // 2. Делегируем создание слоя в LayerManager
    // (Названия методов в вашем LayerManager могут немного отличаться,
    // например createLayer(), addLayer() или generateLayersForNode())
    if (m_layerManager) {
        m_viewManager->forEachView([&](std::shared_ptr<Visualize::IView> view) {
            m_layerManager->createLayer(node, view);
        });
    }

    // 3. Запрашиваем обновление сцены, чтобы слой сразу отрисовался в VTK-окне
    emit sceneUpdateRequested();
    emit markSessionDirty();
}
} // namespace QSpace::Controllers