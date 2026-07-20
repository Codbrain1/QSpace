#include "ColorbarWidget.h"
#include "Physics/DimensionConverter/PhysicalUnits.h"
#include "Visualize/Layers/LayerSettings.h"
#include <QVBoxLayout>
#include <algorithm>
#include <qcolor.h>
#include <qcontainerfwd.h>
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

    // Шкала (легенда)
    m_colorScale    = new QCPColorScale(m_plot);
    QFont labelFont = m_colorScale->axis()->labelFont();
    labelFont.setPointSize(10);
    m_colorScale->axis()->setLabelFont(labelFont);

    QFont tickFont = m_colorScale->axis()->tickLabelFont();
    tickFont.setPointSize(9);
    m_colorScale->axis()->setTickLabelFont(tickFont);

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
}

void ColorBarWidget::setOrientation(bool isVertical) {
    if (!m_colorScale || !m_plot)
        return;

    // 1. Получаем актуальный цвет текста темы
    QColor themeTextColor = this->palette().color(QPalette::WindowText);
    QPen   axisPen(themeTextColor);

    // 2. Меняем политику размеров, тип шкалы и НАСТРАИВАЕМ ОТСТУПЫ (иначе текст обрежется!)
    if (isVertical) {
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_colorScale->setType(QCPAxis::atRight);

        // Отступы для вертикального режима (запас справа под степени 10^x)
        m_plot->plotLayout()->setMargins(QMargins(10, 15, 65, 15));
    } else {
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_colorScale->setType(QCPAxis::atBottom);

        // Отступы для горизонтального режима (запас снизу под подписи осей)
        m_plot->plotLayout()->setMargins(QMargins(15, 10, 15, 45));
    }

    // 3. ПРИНУДИТЕЛЬНО ВОЗВРАЩАЕМ ЦВЕТА И ШРИФТЫ ТЕМЫ ПОСЛЕ СБРОСА
    m_colorScale->axis()->setLabelColor(themeTextColor);
    m_colorScale->axis()->setTickLabelColor(themeTextColor);
    m_colorScale->axis()->setBasePen(axisPen);
    m_colorScale->axis()->setTickPen(axisPen);
    m_colorScale->axis()->setSubTickPen(axisPen);
    m_colorScale->axis()->setTickLabelSide(QCPAxis::lsOutside);

    // Восстанавливаем размеры шрифтов
    QFont labelFont = m_colorScale->axis()->labelFont();
    labelFont.setPointSize(10);
    m_colorScale->axis()->setLabelFont(labelFont);

    QFont tickFont = m_colorScale->axis()->tickLabelFont();
    tickFont.setPointSize(9);
    m_colorScale->axis()->setTickLabelFont(tickFont);

    // 5. Пересчитываем тикеры и диапазоны (вызываем ваш метод обновления)
    handleUpdateValues();

    // 6. Перерисовываем
    m_plot->replot();
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

    if (m_settings == settings)
        return; // уже подключены к этому объекту — не плодим дубликаты

    // отключаем всё, что было подключено к предыдущему m_settings через this
    if (m_settings)
        disconnect(m_settings, nullptr, this, nullptr);

    m_settings = settings;

    connect(m_settings,
            &QSpace::Visualize::Layers::LayerSettings::changed,
            this,
            &ColorBarWidget::handleUpdateValues);

    // на случай уничтожения settings без явного setSettings(nullptr)
    connect(m_settings, &QObject::destroyed, this, [this]() { m_settings = nullptr; });

    handleUpdateValues();
}
void ColorBarWidget::handleUpdateValues() {
    if (!m_settings)
        return;
    QString colorByfield = m_settings->colorByField();
    QString dimension;
    if (colorByfield == "Mass") {
        dimension = "Msun/pc^2";
    } else if (colorByfield == "Density") {
        dimension = "Msun/pc^3";
    } else if (colorByfield == "Energy") {
        dimension = "K";
    } else if (colorByfield == "Velocity") {
        dimension = "km/s";
    }

    m_colorScale->axis()->setLabel(colorByfield + "(" + dimension + ")");

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

    // ---------- стилизация оси (общая для лог/линейного режима) ----------

    if (m_settings->useLogScale()) {
        m_colorScale->axis()->setScaleType(QCPAxis::stLogarithmic);

        // Создаем тикер для лог. шкалы
        QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
        // Шаг логарифма (обычно 10)
        logTicker->setLogBase(10.0);
        logTicker->setTickCount(6);

        m_colorScale->axis()->setTicker(logTicker);
        m_colorScale->axis()->setNumberFormat("eb");
        m_colorScale->axis()->setNumberPrecision(0);

        // безопасный минимум: не константа, а доля от реального максимума,
        // чтобы не "ломать" диапазон при разных порядках величин полей
        double safeMin = (rawMin > 1e-12 * std::max(std::abs(rawMax), 1.0)) ? rawMin : rawMax * 1e-4;
        safeMin        = std::max(safeMin, 1e-12);
        double safeMax = std::max(rawMax, safeMin * 10.0);

        range = QCPRange(safeMin, safeMax);

    } else {
        m_colorScale->axis()->setScaleType(QCPAxis::stLinear);
        m_colorScale->axis()->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));

        m_colorScale->axis()->setNumberFormat("gb");
        m_colorScale->axis()->setNumberPrecision(3);
        range = QCPRange(rawMin, rawMax);
    }

    // 1. Устанавливаем диапазон данных для цветовой карты
    m_colorScale->setDataRange(range);

    // 2. ВАЖНО: Синхронизируем диапазон самой оси, чтобы метки доходили до краев
    m_colorScale->axis()->setRange(range);

    m_plot->replot();
}