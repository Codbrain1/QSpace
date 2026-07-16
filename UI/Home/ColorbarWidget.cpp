#include "ColorbarWidget.h"
#include "Physics/DimensionConverter/PhysicalUnits.h"
#include "Visualize/Layers/LayerSettings.h"
#include <QVBoxLayout>
#include <algorithm>
#include <qcolor.h>
#include <qcustomplot.h>

ColorBarWidget::ColorBarWidget(QWidget* parent) : QWidget(parent) {
    setupPlot();
}

ColorBarWidget::~ColorBarWidget() = default;

void ColorBarWidget::setupPlot() {
    QVBoxLayout* widgetLayout = new QVBoxLayout(this);
    widgetLayout->setContentsMargins(0, 0, 0, 0);

    m_plot = new QCustomPlot(this);
    widgetLayout->addWidget(m_plot);

    QColor themeBgColor   = this->palette().color(QPalette::Window);
    QColor themeTextColor = this->palette().color(QPalette::WindowText);

    m_plot->plotLayout()->clear();
    m_plot->setBackground(QBrush(themeBgColor));

    m_plot->plotLayout()->setMargins(QMargins(0, 0, 0, 0));

    // Шкала (легенда) цвета справа от графика
    m_colorScale = new QCPColorScale(m_plot);
    m_colorScale->setType(QCPAxis::atBottom);
    m_colorScale->setDataRange(QCPRange(0.0, 100.0));

    m_colorScale->axis()->setLabelColor(themeTextColor);
    m_colorScale->axis()->setTickLabelColor(themeTextColor);

    QPen axisPen(themeTextColor);
    m_colorScale->axis()->setBasePen(axisPen);
    m_colorScale->axis()->setTickPen(axisPen);
    m_colorScale->axis()->setSubTickPen(axisPen);
    m_colorScale->axis()->setTickLabelSide(QCPAxis::lsOutside);

    m_plot->plotLayout()->addElement(0, 0, m_colorScale);
    applyGradient(m_currentColorMap);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void ColorBarWidget::applyGradient(const QSpace::Visualize::ColorMap& colorMap) {
    QCPColorGradient gradient;
    gradient.clearColorStops();

    if (colorMap.points.isEmpty()) {
        // Резервный вариант - стандартный градиент QCustomPlot
        gradient = QCPColorGradient::gpJet;
    } else {
        // Сортируем точки по позиции, чтобы порядок вставки стопов был корректным
        QVector<QSpace::Visualize::ColorPoint> sortedPoints = colorMap.points;
        std::sort(sortedPoints.begin(),
                  sortedPoints.end(),
                  [](const QSpace::Visualize::ColorPoint& a, const QSpace::Visualize::ColorPoint& b) {
                      return a.x < b.x;
                  });

        for (const QSpace::Visualize::ColorPoint& point : sortedPoints) {
            const double clampedPos = qBound(0.0, point.x, 1.0);
            gradient.setColorStopAt(clampedPos, QColor::fromRgbF(point.r, point.g, point.b));
        }
    }

    m_colorScale->setGradient(gradient);
    m_currentColorMap = colorMap;

    m_plot->replot();
}

void ColorBarWidget::setColorMap(const QSpace::Visualize::ColorMap& colorMap) {
    applyGradient(colorMap);
}

void ColorBarWidget::setSettings(QSpace::Visualize::Layers::LayerSettings* settings) {
    if (!settings)
        return;

    m_settings = settings;
    handleUpdateValues();
    connect(m_settings,
            &QSpace::Visualize::Layers::LayerSettings::changed,
            this,
            &ColorBarWidget::handleUpdateValues);
}
void ColorBarWidget::handleUpdateValues() {
    if (!m_settings)
        return;

    m_colorScale->axis()->setLabel(m_settings->colorByField());

    QCPRange range;
    double   rawMin = 0;
    double   rawMax = 0;
    if (m_settings->autoRange()) {
        rawMin = m_settings->baseRangeMin();
        rawMax = m_settings->baseRangeMax();
    } else {
        rawMin = m_settings->rangeMin();
        rawMax = m_settings->rangeMax();
    }
    auto   pu          = QSpace::Physics::PhysicalUnits::fromSimParams(3.72, 0.9, 5.0 / 3.0, true);
    double scaleFactor = QSpace::Physics::fieldPhysicalScale(pu, m_settings->colorByField());
    rawMin *= scaleFactor;
    rawMax *= scaleFactor;

    if (m_settings->useLogScale()) {
        m_colorScale->axis()->setScaleType(QCPAxis::stLogarithmic);

        // Создаем тикер для лог. шкалы
        QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
        // Шаг логарифма (обычно 10)
        logTicker->setLogBase(10.0);

        m_colorScale->axis()->setTicker(logTicker);

        double safeMin = (rawMin > 1e-8) ? rawMin : 0.1;
        double safeMax = std::max(safeMin * 10.0, rawMax);
        range          = QCPRange(safeMin, safeMax);
    } else {
        m_colorScale->axis()->setScaleType(QCPAxis::stLinear);
        m_colorScale->axis()->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));

        range = QCPRange(rawMin, rawMax);
    }

    // 1. Устанавливаем диапазон данных для цветовой карты
    m_colorScale->setDataRange(range);

    // 2. ВАЖНО: Синхронизируем диапазон самой оси, чтобы метки доходили до краев
    m_colorScale->axis()->setRange(range);

    applyGradient(m_currentColorMap);
}