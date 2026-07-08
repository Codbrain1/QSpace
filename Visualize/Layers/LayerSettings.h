#pragma once
#include "Common/Structures/ColormapPresets.h"
#include <QColor>
#include <QMetaProperty>
#include <QObject>
#include <QString>
#include <QUuid>
#include <QVariantMap>

namespace QSpace::Visualize::Layers {

class LayerSettings : public QObject {
    Q_OBJECT
    // clang-format off
    Q_PROPERTY(bool    isVisible     READ isVisible     WRITE setVisible       NOTIFY changed)
    Q_PROPERTY(double  opacity       READ opacity       WRITE setOpacity       NOTIFY changed)
    Q_PROPERTY(QString colorByField  READ colorByField  WRITE setColorByField  NOTIFY changed)
    Q_PROPERTY(QUuid   colorMapId    READ colorMapId    WRITE setColorMapId    NOTIFY changed)
    Q_PROPERTY(bool    useLogScale   READ useLogScale   WRITE setUseLogScale   NOTIFY changed)
    Q_PROPERTY(bool    showScalarBar READ showScalarBar WRITE setShowScalarBar NOTIFY changed)
    Q_PROPERTY(double  rangeMin      READ rangeMin      WRITE setRangeMin      NOTIFY changed)
    Q_PROPERTY(double  rangeMax      READ rangeMax      WRITE setRangeMax      NOTIFY changed)
    Q_PROPERTY(double  baseRangeMin  READ baseRangeMin  WRITE setBaseRangeMin  NOTIFY changed)
    Q_PROPERTY(double  baseRangeMax  READ baseRangeMax  WRITE setBaseRangeMax  NOTIFY changed)
    Q_PROPERTY(bool    autoRange     READ autoRange     WRITE setAutoRange     NOTIFY changed)
    Q_PROPERTY(int     zOrder        READ zOrder        WRITE setZOrder        NOTIFY changed)
    Q_PROPERTY(int     blendMode     READ blendMode     WRITE setBlendMode     NOTIFY changed)
    Q_PROPERTY(bool    pickable      READ pickable      WRITE setPickable      NOTIFY changed)

public:
    enum class BlendMode { Alpha = 0, Additive = 1 };

    explicit LayerSettings(QObject* parent = nullptr) : QObject(parent)
    {
        m_colorMapId = Visualize::ColorMapPresets::getPresetByName("Plasma").id;
    }
    virtual ~LayerSettings() = default;

    bool isVisible() const { return m_isVisible; }
    void setVisible(bool v) { if (v != m_isVisible) { m_isVisible = v; emit changed(); } }

    double opacity() const { return m_opacity; }
    void setOpacity(double o) { if (!qFuzzyCompare(o, m_opacity)) { m_opacity = o; emit changed(); } }

    QString colorByField() const { return m_colorByField; }
    void setColorByField(const QString& f) { if (f != m_colorByField) { m_colorByField = f; emit changed(); } }

    QUuid colorMapId() const { return m_colorMapId; }
    void setColorMapId(const QUuid& id) { if (id != m_colorMapId) { m_colorMapId = id; emit changed(); } }

    bool useLogScale() const { return m_useLogScale; }
    void setUseLogScale(bool u) { if (u != m_useLogScale) { m_useLogScale = u; emit changed(); } }

    bool showScalarBar() const { return m_showScalarBar; }
    void setShowScalarBar(bool s) { if (s != m_showScalarBar) { m_showScalarBar = s; emit changed(); } }

    double rangeMin() const { return m_rangeMin; }
    void setRangeMin(double v) { if (!qFuzzyCompare(v, m_rangeMin)) { m_rangeMin = v; emit changed(); } }

    double rangeMax() const { return m_rangeMax; }
    void setRangeMax(double v) { if (!qFuzzyCompare(v, m_rangeMax)) { m_rangeMax = v; emit changed(); } }

    double baseRangeMin() const { return m_baseRangeMin; }
    void setBaseRangeMin(double v) { if (!qFuzzyCompare(v, m_baseRangeMin)) { m_baseRangeMin = v; emit changed(); } }

    double baseRangeMax() const { return m_baseRangeMax; }
    void setBaseRangeMax(double v) { if (!qFuzzyCompare(v, m_baseRangeMax)) { m_baseRangeMax = v; emit changed(); } }

    bool autoRange() const { return m_autoRange; }
    void setAutoRange(bool a) { if (a != m_autoRange) { m_autoRange = a; emit changed(); } }

    int zOrder() const { return m_zOrder; }
    void setZOrder(int z) { if (m_zOrder != z) { m_zOrder = z; emit changed(); } }

    int blendMode() const { return m_blendMode; }
    void setBlendMode(int m) { if (m_blendMode != m) { m_blendMode = m; emit changed(); } }

    bool pickable() const { return m_pickable; }
    void setPickable(bool p) { if (m_pickable != p) { m_pickable = p; emit changed(); } }

    // clang-format on

    QVariantMap toVariantMap() const {
        QVariantMap        map;
        const QMetaObject* mo = metaObject();
        for (int i = QObject::staticMetaObject.propertyCount(); i < mo->propertyCount(); ++i) {
            QMetaProperty prop = mo->property(i);
            map[prop.name()]   = prop.read(this);
        }
        return map;
    }

    void fromVariantMap(const QVariantMap& map) {
        const QMetaObject* mo = metaObject();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            int idx = mo->indexOfProperty(it.key().toUtf8().constData());
            if (idx >= 0)
                mo->property(idx).write(this, it.value());
        }
    }

    virtual QString propertyDisplayName(const QString& propName) const {
        static const QMap<QString, QString> baseNames = {{"isVisible", "Видимость слоя"},
                                                         {"opacity", "Непрозрачность частиц"},
                                                         {"colorByField", "Окрашивать по полю"},
                                                         {"colorMapId", "Цветовая карта"},
                                                         {"useLogScale", "Логарифмическая шкала"},
                                                         {"showScalarBar", "Показывать шкалу"},
                                                         {"rangeMin", "Мин. значение диапазона"},
                                                         {"rangeMax", "Макс. значение диапазона"},
                                                         {"baseRangeMin", "Глобальный минимум"},
                                                         {"baseRangeMax", "Глобальный максимум"},
                                                         {"autoRange", "Авто-диапазон"},
                                                         {"zOrder", "Порядок отображения (Z)"},
                                                         {"blendMode", "Режим смешивания"},
                                                         {"pickable", "Доступен для выбора"}};
        return baseNames.value(propName, propName); // Если не нашли, вернем английское имя
    }

  signals:
    void changed();

  private:
    bool    m_isVisible = true; // видимость слоя
    double  m_opacity   = 1.0;  // непрозрачномсть частиц
    QString m_colorByField;     // имя поля, по которому окрашиваются частицы
    QUuid   m_colorMapId;       // идентификатор цветовой карты, используемой для окрашивания частиц
    bool    m_useLogScale   = true;  // использовать ли логарифмическую шкалу для окрашивания частиц
    bool    m_showScalarBar = true;  // показывать ли цветовую шкалу для окрашивания частиц
    double  m_rangeMin      = 0.0;   // минимальное значение диапазона окрашивания частиц
    double  m_rangeMax      = 100.0; // максимальное значение диапазона окрашивания частиц
    double  m_baseRangeMin  = 0.0;   // глобальное минимальное значение
    double  m_baseRangeMax  = 100.0; // глобальное максимальное значение
    bool    m_autoRange     = true;  // автоматически ли подбирать диапазон окрашивания частиц
    int     m_zOrder        = 0;
    int     m_blendMode     = static_cast<int>(BlendMode::Alpha);
    bool    m_pickable      = true;
};

} // namespace QSpace::Visualize::Layers