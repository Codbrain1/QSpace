
#include "ColorMapManager.h"
#include "Common/Logger/Logger.h"
#include <QList>
#include <quuid.h>
namespace QSpace::Visualize {
ColorMapManager& ColorMapManager::instance() {
    static ColorMapManager _instance;
    return _instance;
}

// Поиск по ID — теперь максимально быстрый
std::optional<ColorMap> ColorMapManager::getMap(const QUuid& id) const {
    if (m_availableMaps.contains(id))
        return m_availableMaps.value(id);
    return std::nullopt;
}

// Для заполнения UI (список всех палитр)
QList<ColorMap> ColorMapManager::getAllMaps() const {
    return m_availableMaps.values();
}

void ColorMapManager::loadCustomMap(const QString& filePath) {
    // ... логика загрузки ...
    // ColorMap newMap = ...
    // m_availableMaps.insert(newMap.id, newMap);
}
void ColorMapManager::saveCustomMap(const ColorMap& map, const QString& filePath) {
    // ... логика сохранения ...
}
void ColorMapManager::AddCustomMap(const ColorMap& map) {
    m_availableMaps.insert(map.id, map);
}
void ColorMapManager::removeCustomMap(const QUuid& id) {
    auto it = m_availableMaps.find(id);
    if (it != m_availableMaps.end() && !it->isPreset) {
        m_availableMaps.erase(it);
    } else {
        qCritical(LogRenderer) << "Attempted to remove a preset or non-existent colormap with ID:" << id;
    }
}

ColorMapManager::ColorMapManager() {
    // При инициализации заполняем карту пресетами
    auto presets = ColorMapPresets::getStandardPresets();
    for (const auto& map : presets) {
        m_availableMaps.insert(map.id, map);
    }
}
bool ColorMapManager::contains(const QUuid& id) {
    return m_availableMaps.contains(id);
}

} // namespace QSpace::Visualize