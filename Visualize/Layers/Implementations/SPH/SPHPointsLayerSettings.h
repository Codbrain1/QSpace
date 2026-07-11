#pragma once
#include "../../LayerSettings.h"

namespace QSpace::Visualize::Layers {
// ============================================================
// Настройки для SPHRendererLayer — splat/resolve пайплайн
// ============================================================
class SPHPointsLayerSettings : public LayerSettings {
    Q_OBJECT
  public:
    enum class KernelType { CubicSpline = 0, Gaussian = 1, Wendland = 2 };

  private:
    // clang-format off
    Q_PROPERTY(float        smoothingRadius         READ smoothingRadius        WRITE setSmoothingRadius        NOTIFY changed)
    Q_PROPERTY(bool         autoSmoothingRadius     READ autoSmoothingRadius    WRITE setAutoSmoothingRadius    NOTIFY changed)
    Q_PROPERTY(KernelType   kernelType              READ kernelType             WRITE setKernelType             NOTIFY changed)
    Q_PROPERTY(float        accumResolutionScale    READ accumResolutionScale   WRITE setAccumResolutionScale   NOTIFY changed)
    Q_PROPERTY(float        densityGamma            READ densityGamma           WRITE setDensityGamma           NOTIFY changed)

public:

    explicit SPHPointsLayerSettings(QObject* parent = nullptr) : LayerSettings(parent) {}

    float smoothingRadius() const { return m_smoothingRadius; }
    void setSmoothingRadius(float r) { if (!qFuzzyCompare(m_smoothingRadius, r)) { m_smoothingRadius = r; emit changed(); } }

    bool autoSmoothingRadius() const { return m_autoSmoothingRadius; }
    void setAutoSmoothingRadius(bool v) { if (m_autoSmoothingRadius != v) { m_autoSmoothingRadius = v; emit changed(); } }


    // выбор формы SPH-кернела — влияет на резкость/мягкость сглаживания
    KernelType kernelType() const { return m_kernelType; }
    void setKernelType(KernelType t) { if (m_kernelType != t) { m_kernelType = t; emit changed(); } }

    // понижение разрешения аккумулирующего FBO (0.5 = вдвое меньше) — для больших N
    float accumResolutionScale() const { return m_accumResolutionScale; }
    void setAccumResolutionScale(float s) { if (!qFuzzyCompare(m_accumResolutionScale, s)) { m_accumResolutionScale = s; emit changed(); } }

    // дополнительная гамма-коррекция контраста после лог-шкалы (1.0 = без изменений)
    float densityGamma() const { return m_densityGamma; }
    void setDensityGamma(float g) { if (!qFuzzyCompare(m_densityGamma, g)) { m_densityGamma = g; emit changed(); } }

    // clang-format on

    // Внутри SPHPointsLayerSettings
    QString propertyDisplayName(const QString& propName) const override {
        static const QMap<QString, QString> sphNames = {
            {"smoothingRadius", "Радиус сглаживания"},
            {"autoSmoothingRadius", "Авто-радиус сглаживания"},
            {"kernelType", "Тип SPH-ядра"},
            {"accumResolutionScale", "Масштаб разрешения FBO"},
            {"densityGamma", "Гамма-коррекция плотности"}};

        if (sphNames.contains(propName)) {
            return sphNames.value(propName);
        }
        // Если это базовое свойство (например, opacity), отдаем базовому классу
        return LayerSettings::propertyDisplayName(propName);
    }

    bool isPropertyEnabled(const QString& propName) const override {
        if (propName == "smoothingRadius") {
            // Если autoSmoothingRadius == true, то обычный радиус менять нельзя
            return !autoSmoothingRadius();
        }
        return LayerSettings::isPropertyEnabled(propName);
    }

    LayerSettings::NumericConstraints propertyConstraints(const QString& propName) const override {
        LayerSettings::NumericConstraints c;
        if (propName == "smoothingRadius") {
            c.min      = 0.001;
            c.max      = 50.0;
            c.step     = 0.01;
            c.decimals = 3; // Шаг 0.01 и 3 знака!
            return c;
        }
        if (propName == "accumResolutionScale") {
            c.min      = 0.1;
            c.max      = 1.0;
            c.step     = 0.05;
            c.decimals = 2;
            return c;
        }
        if (propName == "densityGamma") {
            c.min      = 0.1;
            c.max      = 5.0;
            c.step     = 0.1;
            c.decimals = 2;
            return c;
        }
        return LayerSettings::propertyConstraints(propName); // Возврат к базовым (для диапазонов)
    }

    QString enumValueDisplayName(const QString& propName, const QString& enumKey) const override {
        if (propName == "kernelType") {
            if (enumKey == "CubicSpline")
                return "Кубический сплайн (Cubic Spline)";
            if (enumKey == "Gaussian")
                return "Гауссово ядро (Gaussian)";
            if (enumKey == "Wendland")
                return "Ядро Вендланда (Wendland)";
        }
        return LayerSettings::enumValueDisplayName(propName, enumKey);
    }


  private:
    float      m_smoothingRadius      = 0.05f;
    bool       m_autoSmoothingRadius  = true;
    KernelType m_kernelType           = KernelType::CubicSpline;
    float      m_accumResolutionScale = 1.0f;
    float      m_densityGamma         = 1.0f;
};

} // namespace QSpace::Visualize::Layers