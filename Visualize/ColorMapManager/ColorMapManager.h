#pragma once
#include "Common/Structures/RenderStructures.h"
#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QLinearGradient>
#include <QList>
#include <QMap>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QUuid>
#include <qlogging.h>
#include <quuid.h>


namespace QSpace::Visualize {
class ColorMapManager {
  public:
    static ColorMapManager& instance();

    // Поиск по ID — теперь максимально быстрый
    std::optional<ColorMap> getMap(const QUuid& id) const;

    // Для заполнения UI (список всех палитр)
    QList<ColorMap> getAllMaps() const;

    void  loadCustomMap(const QString& filePath);
    void  saveCustomMap(const ColorMap& map, const QString& filePath);
    void  AddCustomMap(const ColorMap& map);
    void  removeCustomMap(const QUuid& id);
    bool  contains(const QUuid& id);
    QIcon createColorMapIcon(const QSpace::Visualize::ColorMap& map, QSize size = QSize(80, 16));

  private:
    ColorMapManager();
    // Храним мапу для быстрого доступа по UUID
    QMap<QUuid, ColorMap> m_availableMaps;
};
} // namespace QSpace::Visualize