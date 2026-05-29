#include "ObjectRegistry.h"
#include "Common/Logger/Logger.h"
#include "Common/Structures/CoreStructures.h"
#include <qcontainerfwd.h>
#include <qloggingcategory.h>
#include <qobject.h>
#include <quuid.h>
#include <memory>

namespace QSpace::Core {
ObjectRegistry::ObjectRegistry(QObject* parent) : QObject(parent) {
}

void ObjectRegistry::registerNode(std::shared_ptr<DataNode> node) {
    if (!node)
        return;
    if (m_nodes.contains(node->id)) {
        qCWarning(LogCore) << "Registry: Attempt to register duplicate node ID:" << node->id;
        return;
    }
    m_nodes.insert(node->id, node);
    if (node->data != nullptr) {
        touchNodeInMemory(node->id);
    }
    emit nodeAdded(node);
    qCInfo(LogCore) << "Registry: Node registered " << node->label << " " << node->id;
}

void ObjectRegistry::registerContainer(std::shared_ptr<DataContainer> container) {
    if (!container)
        return;
    if (m_containers.contains(container->id)) {
        qCWarning(LogCore) << "Registry: Attempt to register duplicate container ID:"
                           << container->id;
        return;
    }
    m_containers.insert(container->id, container);
    for (auto& comp : container->components) {
        registerNode(comp);
    }
    emit containerAdded(container);
    qCDebug(LogCore) << "Registry: Container registered " << container->name << " "
                     << container->id;
}

void ObjectRegistry::registerExperiment(std::shared_ptr<Experiment> experiment) {
    if (!experiment)
        return;
    if (m_experiments.contains(experiment->id)) {
        qCWarning(LogCore) << "Registry: Attempt to register duplicate experiment ID:"
                           << experiment->id;
        return;
    }
    m_experiments.insert(experiment->id, experiment);
    for (auto& snap : experiment->snapshots) {
        registerContainer(snap);
    }
    emit experimentAdded(experiment);
    qCDebug(LogCore) << "Registry: Experiment registered " << experiment->name << " "
                     << experiment->id;
}

// ObjectRegistry.cpp
void ObjectRegistry::registerNodeWithGrouping(std::shared_ptr<DataNode> node,
                                              const QUuid&              experimentId) {
    if (!node)
        return;
    // 1. Регистрируем атомарную ноду
    registerNode(node);
    auto targetExperiment = m_experiments.value(experimentId);
    if (!targetExperiment) {
        qCWarning(LogCore) << "Registry: Experiment ID not found for grouping:" << experimentId;
        return;
    }
    // 4. Ищем или создаем Снапшот внутри найденного эксперимента
    double                         ts             = node->stats.timestamp;
    const double                   eps            = 1e-5;
    std::shared_ptr<DataContainer> targetSnapshot = nullptr;
    for (const auto& snap : targetExperiment->snapshots) {
        if (std::abs(snap->timestamp - ts) < eps) {
            targetSnapshot = snap;
            break;
        }
    }

    if (!targetSnapshot) {
        QString snapName = QString("Snapshot (t = %1)").arg(ts, 0, 'f', 5);
        targetSnapshot   = std::make_shared<DataContainer>(snapName, ts);

        targetExperiment->addSnapshot(targetSnapshot); // Привязываем к эксперименту
        registerContainer(targetSnapshot); // Регистрируем в m_containers (emit containerAdded)
    }

    // 5. Добавляем ноду в компоненты снапшота
    targetSnapshot->addComponent(node);
}

std::shared_ptr<DataNode> ObjectRegistry::getNode(const QUuid& id) const {
    return m_nodes.value(id, nullptr);
}

std::shared_ptr<DataContainer> ObjectRegistry::getContainer(const QUuid& id) const {
    return m_containers.value(id, nullptr);
}

void ObjectRegistry::removeObject(const QUuid& id) {
    // =========================================================================
    // СЦЕНАРИЙ 1: УДАЛЕНИЕ ЭКСПЕРИМЕНТА (Самый верхний уровень)
    // =========================================================================
    if (m_experiments.contains(id)) {
        auto experiment = m_experiments.take(id);
        qCInfo(LogCore) << "Registry: Removing entire experiment:" << experiment->name;

        for (const auto& snap : experiment->snapshots) {
            if (!snap)
                continue;

            // Удаляем ноды этого конкретного снапшота
            for (const auto& node : snap->components) {
                if (!node)
                    continue;

                // Стираем ноду из глобального реестра и LRU (прямые O(1) операции)
                m_nodes.remove(node->id);
                if (m_lruMap.contains(node->id)) {
                    m_lruList.erase(m_lruMap[node->id]);
                    m_lruMap.remove(node->id);
                }
                emit objectRemoved(node->id);
            }

            // Удаляем сам снапшот из реестра контейнеров
            m_containers.remove(snap->id);
            emit objectRemoved(snap->id);
        }

        emit objectRemoved(id); // Оповещаем UI об удалении эксперимента
        return;
    }

    // =========================================================================
    // СЦЕНАРИЙ 2: УДАЛЕНИЕ ОДИНОЧНОГО КОНТЕЙНЕРА (Снапшота)
    // =========================================================================
    if (m_containers.contains(id)) {
        auto container = m_containers.take(id);
        qCInfo(LogCore) << "Registry: Removing snapshot container:" << container->name;

        // 1. Сначала убираем ссылку на этот снапшот из его родительского эксперимента
        for (auto& exp : m_experiments) {
            auto It = std::find(exp->snapshots.begin(), exp->snapshots.end(), container);
            if (It != exp->snapshots.end()) {
                exp->snapshots.erase(It);
                break; // Снапшот принадлежит только одному эксперименту
            }
        }

        // 2. Точечно удаляем только те ноды, которые принадлежали этому снапшоту
        for (const auto& node : container->components) {
            if (!node)
                continue;

            m_nodes.remove(node->id);
            if (m_lruMap.contains(node->id)) {
                m_lruList.erase(m_lruMap[node->id]);
                m_lruMap.remove(node->id);
            }
            emit objectRemoved(node->id);
        }

        emit objectRemoved(id);
        return;
    }

    // =========================================================================
    // СЦЕНАРИЙ 3: УДАЛЕНИЕ ОДИНОЧНОЙ НОДЫ (Например, пользователь удалил слой газа)
    // =========================================================================
    if (m_nodes.contains(id)) {
        // Тут полный перебор контейнеров оправдан, т.к. мы не знаем, где именно лежит нода.
        // Но это работает быстро, потому что вызывается редко и НЕ рекурсивно!
        for (auto& container : m_containers) {
            auto& comps = container->components;
            auto  it    = std::remove_if(
                comps.begin(),
                comps.end(),
                [&id](const std::shared_ptr<DataNode>& n) { return n && n->id == id; });
            if (it != comps.end()) {
                comps.erase(it, comps.end());
                break; // Нода уникальна и лежит в одном конкретном временном шаге
            }
        }

        // Чистим LRU
        if (m_lruMap.contains(id)) {
            m_lruList.erase(m_lruMap[id]);
            m_lruMap.remove(id);
        }

        m_nodes.remove(id);
        emit objectRemoved(id);
        qCInfo(LogCore) << "Registry: Single node removed:" << id;
    }
}

std::shared_ptr<DataContainer> ObjectRegistry::findContainerByName(const QString& name) const {
    for (auto container : m_containers) {
        if (container->name == name) {
            return container;
        }
    }
    return nullptr;
}

void ObjectRegistry::touchNodeInMemory(const QUuid& id) {
    if (m_lruMap.contains(id)) {
        m_lruList.erase(m_lruMap[id]);
    }
    m_lruList.push_front(id);
    m_lruMap[id] = m_lruList.begin();

    if (m_lruList.size() > m_cacheCapacity) {
        QUuid oldestId   = m_lruList.back();
        auto  oldestNode = m_nodes.value(oldestId);

        if (oldestNode && oldestNode->data != nullptr) {
            oldestNode->data = nullptr; // Освобождаем память VTK
            qCInfo(LogCore) << "LRU Cache: Evicted data for node" << oldestNode->label;

            // Оповещаем UI, что статус памяти изменился
            emit nodeDataUpdated(oldestId);
        }

        m_lruMap.remove(oldestId);
        m_lruList.pop_back();
    }
}

// Реализация ленивой загрузки (Вызывать из Layer::update или PlaybackController)
std::shared_ptr<DataNode> ObjectRegistry::getOrLoadNodeData(const QUuid& id) {
    auto node = m_nodes.value(id, nullptr);
    if (!node)
        return nullptr;

    bool wasLoadedJustNow = false;

    // Если данных в ОЗУ нет — читаем с диска
    if (node->data == nullptr) {
        qCInfo(LogCore) << "LRU Cache: Lazy loading heavy VTK data for" << node->label;
        // Здесь должна быть реальная логика загрузки данных из файла

        wasLoadedJustNow = true;
    }

    if (node->data != nullptr) {
        touchNodeInMemory(id); // Двигаем в начало кэша, возможно вытесняя старые

        if (wasLoadedJustNow) {
            emit nodeDataUpdated(id); // Оповещаем UI, что данные теперь "In RAM"
        }
    }

    return node;
}

// НОВЫЙ
void ObjectRegistry::updateNodeData(const QUuid& id, vtkSmartPointer<vtkDataSet> dataSet) {
    auto node = m_nodes.value(id);
    if (!node)
        return;

    node->data = dataSet;

    if (dataSet != nullptr) {
        // Данные вернулись в ОЗУ -> активируем/освежаем ноду в LRU
        touchNodeInMemory(id);
    } else {
        // Если данные принудительно занулили извне — убираем из LRU track'ера
        if (m_lruMap.contains(id)) {
            m_lruList.erase(m_lruMap[id]);
            m_lruMap.remove(id);
        }
    }

    // emit nodeUpdated(node); // Если UI нужно перерисовать ноду (например, иконка "загружено")
}
} // namespace QSpace::Core