#pragma once
#include "../../LayerSettings.h"

namespace QSpace::Visualize::Layers {
// ============================================================
// Настройки для SPHRendererLayer — splat/resolve пайплайн
// ============================================================
class SPHPointsLayerSettings : public LayerSettings {
    Q_OBJECT
    // clang-format off
    Q_PROPERTY(float smoothingRadius     READ smoothingRadius     WRITE setSmoothingRadius     NOTIFY changed)
    Q_PROPERTY(bool  autoSmoothingRadius READ autoSmoothingRadius WRITE setAutoSmoothingRadius NOTIFY changed)
    Q_PROPERTY(int   kernelType          READ kernelType          WRITE setKernelType          NOTIFY changed)
    Q_PROPERTY(float accumResolutionScale READ accumResolutionScale WRITE setAccumResolutionScale NOTIFY changed)
    Q_PROPERTY(float densityGamma        READ densityGamma        WRITE setDensityGamma        NOTIFY changed)

public:
    enum class KernelType { CubicSpline = 0, Gaussian = 1, Wendland = 2 };

    explicit SPHPointsLayerSettings(QObject* parent = nullptr) : LayerSettings(parent) {}

    float smoothingRadius() const { return m_smoothingRadius; }
    void setSmoothingRadius(float r) { if (!qFuzzyCompare(m_smoothingRadius, r)) { m_smoothingRadius = r; emit changed(); } }

    bool autoSmoothingRadius() const { return m_autoSmoothingRadius; }
    void setAutoSmoothingRadius(bool v) { if (m_autoSmoothingRadius != v) { m_autoSmoothingRadius = v; emit changed(); } }


    // выбор формы SPH-кернела — влияет на резкость/мягкость сглаживания
    int kernelType() const { return m_kernelType; }
    void setKernelType(int t) { if (m_kernelType != t) { m_kernelType = t; emit changed(); } }

    // понижение разрешения аккумулирующего FBO (0.5 = вдвое меньше) — для больших N
    float accumResolutionScale() const { return m_accumResolutionScale; }
    void setAccumResolutionScale(float s) { if (!qFuzzyCompare(m_accumResolutionScale, s)) { m_accumResolutionScale = s; emit changed(); } }

    // дополнительная гамма-коррекция контраста после лог-шкалы (1.0 = без изменений)
    float densityGamma() const { return m_densityGamma; }
    void setDensityGamma(float g) { if (!qFuzzyCompare(m_densityGamma, g)) { m_densityGamma = g; emit changed(); } }

    // clang-format on

  private:
    float m_smoothingRadius      = 0.05f;
    bool  m_autoSmoothingRadius  = true;
    int   m_kernelType           = static_cast<int>(KernelType::CubicSpline);
    float m_accumResolutionScale = 1.0f;
    float m_densityGamma         = 1.0f;
};

} // namespace QSpace::Visualize::Layers