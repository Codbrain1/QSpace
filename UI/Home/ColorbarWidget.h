#pragma once

#include "Common/Structures/ColormapPresets.h"
#include "qcustomplot.h"
#include <QColor>
#include <QString>
#include <QUuid>
#include <QVector>
#include <QWidget>

// Forward-declare реального класса настроек слоя (замените на ваш include)
namespace QSpace {
namespace Visualize {
namespace Layers {
class LayerSettings;
}
} // namespace Visualize
} // namespace QSpace

class ColorBarWidget : public QWidget {
    Q_OBJECT

  public:
    explicit ColorBarWidget(QWidget* parent = nullptr);
    ~ColorBarWidget() override;

    // Задать цветовую палитру (градиент) для отображения
    void setColorMap(const QSpace::Visualize::ColorMap& colorMap);

    // Применить настройки слоя (диапазон значений, палитра, видимость и т.д.)
    void setSettings(QSpace::Visualize::Layers::LayerSettings* settings);
    void setOrientation(bool isVertical);
    // Доступ к нижележащему QCustomPlot, если понадобится тонкая настройка снаружи
    QCustomPlot* plot() const {
        return m_plot;
    }

  private:
    void setupPlot();
    void applyGradient(const QSpace::Visualize::ColorMap& colorMap);
  private slots:
    void handleUpdateValues();

  private:
    QCustomPlot*   m_plot       = nullptr;
    QCPColorScale* m_colorScale = nullptr;

    QSpace::Visualize::ColorMap               m_currentColorMap;
    QSpace::Visualize::Layers::LayerSettings* m_settings = nullptr;
};