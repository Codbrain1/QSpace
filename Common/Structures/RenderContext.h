#pragma once
#include <QMatrix4x4>
#include <QSize>
#include <QVector3D>

namespace QSpace::Visualize {
// нужен всем рендерерам
struct RenderContext {
    QMatrix4x4 viewMatrix;
    QMatrix4x4 projMatrix;
    QMatrix4x4 mvp; // projMatrix * viewMatrix, посчитан один раз за кадр
    QVector3D  cameraPos;
    QSize      viewportPx;
    float      pixelsPerWorldUnit = 1.0f; // актуально для ортографии
    bool       orthographic       = true;
};

} // namespace QSpace::Visualize