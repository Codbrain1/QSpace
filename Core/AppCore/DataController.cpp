#include "DataController.h"

// Подключение внутренних менеджеров ядра
#include "Common/Interfaces/IRenderLayer.h"
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
#include <qfileinfo.h>
#include <qloggingcategory.h>
#include <qobjectdefs.h>
#include <optional>

namespace QSpace::Core::Controllers {

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
    connect(m_dataManager, &Core::DataManager::fileReady, this, &DataController::handleFileReady);

    connect(m_dataManager,
            &Core::DataManager::ioStarted,
            this,
            [this](const QUuid& taskId, const QString&, int total) {
                m_activeTasks[taskId].totalFiles = total;
            });

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
    connect(m_objectRegistry,
            &Core::ObjectRegistry::dataLoadRequested,
            this,
            &DataController::onRequestDataLoad);
}

void DataController::importFiles(const QStringList&            paths,
                                 Core::ModelingProgrammVersion version,
                                 const QUuid&                  targetExperimentId) {
    if (paths.isEmpty())
        return;
    QUuid experimentId;
    if (!targetExperimentId.isNull() && m_objectRegistry) {
        // Проверяем, существует ли такой эксперимент.
        // Если пользователь кликнул на обычную ноду, getExperiment вернет nullptr.
        if (m_objectRegistry->getExperiment(targetExperimentId) != nullptr) {
            experimentId = targetExperimentId;
        } else {
            // Опционально: Если у вас есть логика поиска родительского эксперимента
            // по ID вложенной ноды, ее можно добавить сюда.
            qCInfo(LogCore) << "Выделенный элемент не является экспериментом. Данные будут "
                               "импортированы независимо.";
        }
    }
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
        QUuid taskId                     = m_dataManager->importDataAsync(paths[0], scheme);
        m_activeTasks[taskId].totalFiles = 1;
        if (!experimentId.isNull()) {
            m_taskToExperiment.insert(taskId, experimentId);
        }
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
        QUuid batchTaskId                     = m_dataManager->importBatchDataAsync(tasks);
        m_activeTasks[batchTaskId].totalFiles = paths.size();
        if (!experimentId.isNull()) {
            m_taskToExperiment.insert(batchTaskId, experimentId);
        }
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

void DataController::importExperiment(const QStringList&            filePaths,
                                      const QString&                experimentName,
                                      Core::ModelingProgrammVersion version) {
    if (filePaths.isEmpty() || experimentName.isEmpty() || !m_dataManager || !m_objectRegistry) {
        return;
    }
    auto experiment = std::make_shared<Core::Experiment>(experimentName);
    experiment->id  = QUuid::createUuid();
    m_objectRegistry->registerExperiment(experiment);
    importFiles(filePaths, version, experiment->id);
}

QList<std::shared_ptr<QSpace::Core::Experiment>> DataController::getExperiments() const {
    return m_objectRegistry->getAllExperiments();
}

void DataController::removeNodeObject(const QUuid& id) {
    // Контроллер занимается только структурами данных.
    // ViewController поймает этот сигнал реактивно и зачистит VTK слои в окнах.
    m_objectRegistry->removeObject(id);
    emit markSessionDirty();
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

std::optional<QUuid> DataController::getExperimentIdByNodePath(const QString& nodePath) const {
    // 1. Получаем все эксперименты из реестра
    auto experiments = m_objectRegistry->getAllExperiments();

    // 2. Проходим по каждому эксперименту и его нодам, сравнивая пути
    for (const auto& experiment : experiments) {
        for (const auto& snapshot : experiment->snapshots) {
            for (const auto& node : snapshot->components) {
                if (node && node->path == nodePath) {
                    return experiment->id; // Возвращаем ID эксперимента, если найдено совпадение
                }
            }
        }
    }

    return std::nullopt; // Если совпадений не найдено, возвращаем nullopt
}

std::optional<QUuid> DataController::getNodeIdByFilePath(const QString& filePath) const {
    auto nodes = m_objectRegistry->getAllNodes();
    for (const auto& node : nodes) {
        if (node && node->path == filePath) {
            return node->id;
        }
    }
    return std::nullopt;
}

std::shared_ptr<Core::DataNode> DataController::getNodeById(const QUuid& nodeId) {
    return m_objectRegistry->getNode(nodeId);
}

QUuid DataController::getNodePaletteId(const QUuid& nodeId) {
    auto node = m_objectRegistry->getNode(nodeId);
    return node ? node->masterSettings->colorMapId : QUuid();
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

void DataController::prepareNodesForRestoration(
    const QMap<QString, Session::DataNodeState>& restoringNodes) {
    m_restoringNodes = restoringNodes;
}

// ---------------------------------------------------------
// @SECTION: Реакция на UI (Слайдер и Дерево)
// ---------------------------------------------------------
void DataController::handleTimeSliderValueChanged(int index, bool isPreview) {
    if (index < 0 || index >= m_timeSliderSnapshots.size())
        return;
    // 1. Гасим предыдущий активный кадр (чтобы не наслаивать геометрию)
    if (!m_currentActiveSnapshotId.isNull()) {
        auto oldSnapshot = m_objectRegistry->getSnapshot(m_currentActiveSnapshotId);
        if (oldSnapshot) {
            m_layerManager->setSnapshotVisibility(oldSnapshot, false);
        }
    }
    // 2. Делаем текущий (выбранный) кадр активным
    QUuid targetSnapshotId = m_timeSliderSnapshots[index].targetSnapshot;
    auto  targetSnapshot   = m_objectRegistry->getSnapshot(targetSnapshotId);

    m_currentActiveSnapshotId = targetSnapshotId;
    if (targetSnapshot) {
        // Отобразятся только те слои, у которых стоит галочка видимости
        m_layerManager->setSnapshotVisibility(targetSnapshot, true);
    }

    // 3. Формируем "скользящее окно" кэширования: [index - 7 ... index ... index + 7]
    int windowRadius = 7;
    int startIdx     = std::max(0, index - windowRadius);
    int endIdx = std::min(static_cast<int>(m_timeSliderSnapshots.size() - 1), index + windowRadius);

    for (int i = startIdx; i <= endIdx; ++i) {
        auto snap = m_objectRegistry->getSnapshot(m_timeSliderSnapshots[i].targetSnapshot);
        if (!snap)
            continue;

        for (const auto& node : snap->components) {
            // ФИЛЬТР: Обрабатываем только ту компоненту, которую пользователь хочет видеть
            if (!node || !node->masterSettings->isVisible)
                continue;

            if (node->data == nullptr) {
                // ДАННЫХ НЕТ В ОЗУ
                // Грузим данные с диска только если это медленное движение/остановка (!isPreview)
                // ИЛИ если это непосредственно текущий кадр, на который смотрит пользователь.
                if (!isPreview || i == index) {
                    QMetaObject::invokeMethod(this, "onRequestDataLoad", Q_ARG(QUuid, node->id));
                }
            } else {
                // ДАННЫЕ УЖЕ В ОЗУ
                // Проверяем, есть ли для них физический VTK-слой. Если нет — создаем.
                auto layers = m_layerManager->getLayersForNode(node->id);
                if (layers.isEmpty()) {
                    createLayerForNode(node->id);
                }
            }
        }
    }
}

// Измените сигнатуру метода (не забудьте добавить bool isPreview в DataController.h!)
void DataController::activateSnapshotInternal(const QUuid& snapshotId, bool isPreview) {
    if (m_currentActiveSnapshotId == snapshotId)
        return;

    // 1. ВЫКЛЮЧАЕМ ПРЕДЫДУЩИЙ КАДР
    if (!m_currentActiveSnapshotId.isNull()) {
        auto oldSnapshot = m_objectRegistry->getSnapshot(m_currentActiveSnapshotId);
        if (oldSnapshot) {
            // Вместо тяжелого updateNodeMasterSettings используем setSnapshotVisibility
            m_layerManager->setSnapshotVisibility(oldSnapshot, false);
        }
    }

    // 2. ВКЛЮЧАЕМ НОВЫЙ КАДР
    auto newSnapshot = m_objectRegistry->getSnapshot(snapshotId);
    if (!newSnapshot)
        return;

    if (!isPreview) {
        // ЧЕСТНАЯ ЗАГРУЗКА (Слайдер отпустили)
        for (const auto& node : newSnapshot->components) {
            if (!node)
                continue;
            if (m_objectRegistry->getOrLoadNodeData(node->id)) {
                node->masterSettings->isVisible = true;
                m_layerManager->updateNodeMasterSettings(node->id);
            }
        }
    } else {
        // БЫСТРОЕ ПРЕВЬЮ (Слайдер тянут)
        // Включаем только то, что УЖЕ есть в оперативной памяти (data != nullptr)
        for (const auto& node : newSnapshot->components) {
            if (!node)
                continue;
            if (node->data != nullptr) { // Проверяем без вызова getOrLoadNodeData!
                node->masterSettings->isVisible = true;
                m_layerManager->updateNodeMasterSettings(node->id);
            }
        }
    }

    m_currentActiveSnapshotId = snapshotId;
    emit sceneUpdateRequested(); // Теперь этот сигнал сработает в MainWindow!
}

void DataController::onNodeSelectionActivated(const QUuid& nodeId) {
    auto node = m_objectRegistry->getNode(nodeId);
    if (!node)
        return;

    // Логика идентична таймлайну
    if (m_objectRegistry->getOrLoadNodeData(nodeId)) {
        node->masterSettings->isVisible = true;
        m_layerManager->updateNodeMasterSettings(nodeId);
        emit sceneUpdateRequested();
    }
}

void DataController::handleTargetExperimentVisualizeChanged(const QUuid& experimentId) {
    if (!m_objectRegistry->containsExperiment(experimentId)) {
        qCWarning(LogCore) << "Experiment with QUuid: " << experimentId.toString()
                           << " don't exist!";
        return;
    }

    m_currentVisualizeExperimentId = experimentId;
    auto experiment_ptr            = m_objectRegistry->getExperiment(experimentId);
    auto size                      = experiment_ptr->snapshots.size();
    m_timeSliderSnapshots.clear();
    m_timeSliderSnapshots.reserve(size);
    for (const auto& snapshot : experiment_ptr->snapshots) {
        m_timeSliderSnapshots.emplace_back(snapshot->id, snapshot->timestamp);
    }
    std::sort(m_timeSliderSnapshots.begin(), m_timeSliderSnapshots.end());
    emit snapshotsListSizeChanged(size);
}

void DataController::handleFileReady(const QUuid& taskId, QSpace::IO::ReadResult result) {
    // 1. Первичная обработка ошибок ввода-вывода
    if (!result.isSuccess()) {
        qCWarning(LogCore) << "Error loading file:" << result.errMessage;
        m_restoringNodes.remove(result.path);

        // Если упала ленивая загрузка, обязательно снимаем блокировку от гонки данных
        QUuid failedNodeId = m_activeTasks.value(taskId).targetNodeId;
        if (!failedNodeId.isNull()) {
            m_loadingNodes.remove(failedNodeId);
        }
        return;
    }

    // Извлекаем ID целевой ноды из карточки задачи (если задача была создана методом
    // requestDataLoad)
    QUuid targetNodeId = m_activeTasks.value(taskId).targetNodeId;

    // =================================================================
    // СЦЕНАРИЙ А: ЛЕНИВАЯ ЗАГРУЗКА (Запись уже есть в ObjectRegistry)
    // =================================================================
    if (!targetNodeId.isNull()) {
        m_loadingNodes.remove(targetNodeId); // Снимаем блокировку, данные в ОЗУ

        // Передаем тяжелый vtkDataSet в ObjectRegistry (он займется LRU и пересчетом stats)
        m_objectRegistry->updateNodeData(targetNodeId, result.data, result.timestamp);

        // Активируем слой и запрашиваем рендер сцены
        auto node = m_objectRegistry->getNode(targetNodeId);
        if (node) {
            node->masterSettings->isVisible = true;
            if (m_layerManager) {
                m_layerManager->updateNodeMasterSettings(targetNodeId);
            }
            qCInfo(LogCore) << "Lazy load completed and geometry attached for:" << node->label;
        }

        emit sceneUpdateRequested();
        return; // Выходим, так как структуру сущностей создавать не нужно
    }

    // =================================================================
    // СЦЕНАРИЙ Б: ПЕРВИЧНЫЙ ИМПОРТ (Создание новой записи по метаданным)
    // =================================================================
    QString               fileName = QFileInfo(result.path).fileName();
    Visualize::EntityType type     = IO::Utils::getEntityType(fileName);

    // Создаем «каркас» узла (result.data здесь пустой, т.к. прочитан только заголовок)
    auto node = std::make_shared<Core::DataNode>(
        result.data,
        fileName,
        QSpace::Physics::Math::calculateTimestamp(result.timestamp),
        type);

    // Проверяем: мы восстанавливаем сохраненный проект или импортируем новые файлы?
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
        // Обычный новый импорт с диска
        node->path             = result.path;
        node->format           = result.format;
        node->label            = fileName;
        node->type             = type;
        node->scheme           = result.scheme;
        node->stats.pointCount = result.pointCount; // Записываем размер из прочитанного заголовка

        // Если файлы идут пачкой (часть эксперимента), скрываем их, чтобы не перегрузить сцену.
        // Одиночные файлы показываем сразу.
        int totalInThisTask             = m_activeTasks.value(taskId).totalFiles;
        node->masterSettings->isVisible = (totalInThisTask == 1);

        emit markSessionDirty();
    }

    qCInfo(LogCore) << "DataController::handleFileReady - Скелет ноды создан для:" << fileName
                    << "| Заявлено точек:" << node->stats.pointCount;

    // Флаг: принадлежит ли файл какому-либо упорядоченному эксперименту?
    bool isInsideExperiment = m_taskToExperiment.contains(taskId);

    // 4. Регистрация в ObjectRegistry (Вся логика группировки инкапсулирована там!)
    if (isInsideExperiment) {
        QUuid experimentId = m_taskToExperiment.value(taskId);
        // Реестр сам найдет/создаст нужный Snapshot внутри Эксперимента по timestamp ноды
        m_objectRegistry->registerNodeToExperiment(node, experimentId);
        qCDebug(LogCore) << "Нода" << fileName
                         << "делегирована реестру для интеграции в эксперимент:" << experimentId;
    } else {
        // Одиночный независимый импорт
        m_objectRegistry->registerNode(node);
        qCDebug(LogCore) << "Нода" << fileName << "зарегистрирована на верхнем уровне реестра";
    }

    // 5. Создание визуальных слоев в VTK-окнах (Только для одиночных независимых файлов!)
    // Для файлов внутри экспериментов слои создаются реактивно (при движении слайдера таймлайна)
    if (!isInsideExperiment) {
        if (m_viewManager && m_layerManager) {
            m_viewManager->forEachView([&](std::shared_ptr<Visualize::IView> view) {
                m_layerManager->createLayer(node, view);
            });
        }
    }
}

void DataController::onRequestDataLoad(const QUuid& nodeId) {
    if (m_loadingNodes.contains(nodeId))
        return;

    auto node = m_objectRegistry->getNode(nodeId);
    if (!node || node->path.isEmpty())
        return;

    m_loadingNodes.insert(nodeId);

    QUuid taskId =
        m_dataManager->importDataAsync(node->path, node->scheme, IO::ImportRole::FullData);

    // Сразу регистрируем задачу и привязываем к ней ноду
    m_activeTasks[taskId] = TaskInfo{nodeId, 1};

    qCInfo(LogCore) << "Lazy loading started for:" << node->label;
}

QString DataController::extractGroupName(const QString& filename) {
    // Регулярное выражение для группировки шагов симуляций (например: snap_001, step-500)
    static QRegularExpression regex(R"((snap(shot)?|step)[_\-]?\d+)",
                                    QRegularExpression::CaseInsensitiveOption);
    auto                      match = regex.match(filename);
    return match.hasMatch() ? match.captured(0) : QString();
}

std::shared_ptr<Core::Snapshot> DataController::findOrCreateSnapshot(const QString& groupName) {
    auto existing = m_objectRegistry->findSnapshotByName(groupName);
    if (existing)
        return existing;

    auto newContainer = std::make_shared<Core::Snapshot>(groupName);
    m_objectRegistry->registerSnapshot(newContainer);
    return newContainer;
}



} // namespace QSpace::Core::Controllers