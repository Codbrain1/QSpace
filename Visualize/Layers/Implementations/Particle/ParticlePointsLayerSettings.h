#pragma once
#include "../../LayerSettings.h"

namespace QSpace::Visualize::Layers {
// ============================================================
// Настройки для ParticleRendererLayer — обычные точки-частицы
// ============================================================
class ParticlePointsLayerSettings : public LayerSettings {
    Q_OBJECT
    // clang-format off
    Q_PROPERTY(float pointSizePx     READ pointSizePx     WRITE setPointSizePx     NOTIFY changed)
    Q_PROPERTY(bool  sizeByField     READ sizeByField     WRITE setSizeByField     NOTIFY changed)
    Q_PROPERTY(float minPointSizePx  READ minPointSizePx  WRITE setMinPointSizePx  NOTIFY changed)
    Q_PROPERTY(float maxPointSizePx  READ maxPointSizePx  WRITE setMaxPointSizePx  NOTIFY changed)
    Q_PROPERTY(bool  shadeAsSphere   READ shadeAsSphere   WRITE setShadeAsSphere   NOTIFY changed)

public:
    explicit ParticlePointsLayerSettings(QObject* parent = nullptr) : LayerSettings(parent) {}

    float pointSizePx() const { return m_pointSizePx; }
    void setPointSizePx(float s) { if (!qFuzzyCompare(m_pointSizePx, s)) { m_pointSizePx = s; emit changed(); } }

    // если true — размер точки масштабируется по значению colorByField (min..max диапазон)
    bool sizeByField() const { return m_sizeByField; }
    void setSizeByField(bool v) { if (m_sizeByField != v) { m_sizeByField = v; emit changed(); } }

    float minPointSizePx() const { return m_minPointSizePx; }
    void setMinPointSizePx(float s) { if (!qFuzzyCompare(m_minPointSizePx, s)) { m_minPointSizePx = s; emit changed(); } }

    float maxPointSizePx() const { return m_maxPointSizePx; }
    void setMaxPointSizePx(float s) { if (!qFuzzyCompare(m_maxPointSizePx, s)) { m_maxPointSizePx = s; emit changed(); } }

    // круглый спрайт с псевдо-затенением (sqrt(1-r^2)) вместо плоского квадрата
    bool shadeAsSphere() const { return m_shadeAsSphere; }
    void setShadeAsSphere(bool v) { if (m_shadeAsSphere != v) { m_shadeAsSphere = v; emit changed(); } }

    // Внутри ParticlePointsLayerSettings
    QString propertyDisplayName(const QString& propName) const override {
    static const QMap<QString, QString> particleNames = {
        {"pointSizePx",     "Размер точки (px)"},
        {"sizeByField",     "Масштаб от поля"},
        {"minPointSizePx",  "Мин. размер точки"},
        {"maxPointSizePx",  "Макс. размер точки"},
        {"shadeAsSphere",   "Сферическое затенение"}
    };
    if (particleNames.contains(propName)) return particleNames.value(propName);
    return LayerSettings::propertyDisplayName(propName);
}

    // clang-format on
  private:
    float m_pointSizePx    = 4.0f;
    bool  m_sizeByField    = false;
    float m_minPointSizePx = 1.0f;
    float m_maxPointSizePx = 12.0f;
    bool  m_shadeAsSphere  = true;
};

} // namespace QSpace::Visualize::Layers