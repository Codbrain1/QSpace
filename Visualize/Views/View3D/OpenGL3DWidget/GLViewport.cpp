#include "GLViewport.h"
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>
#include <cmath>


namespace QSpace::Visualize::Views::View3D {

// ... конструктор/деструктор без изменений (см. предыдущее сообщение) ...

void GLViewport::attachRenderLayer(const QUuid&                                   layerId,
                                   std::shared_ptr<Visualize::IOpenGLRenderLayer> layer) {
    if (!layer)
        return;

    if (isValid()) {
        makeCurrent();
        layer->initializeGL(this);
        doneCurrent();
    }
    m_layers.insert(layerId, layer);
    m_needsCameraFit = true;
    update();
}

void GLViewport::detachRenderLayer(const QUuid& layerId) {
    auto it = m_layers.find(layerId);
    if (it == m_layers.end())
        return;

    if (isValid()) {
        makeCurrent();
        it.value()->releaseGL(this);
        doneCurrent();
    }
    m_layers.erase(it);
    update();
}

void GLViewport::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_PROGRAM_POINT_SIZE);
    glClearColor(float(m_backgroundColor.redF()),
                 float(m_backgroundColor.greenF()),
                 float(m_backgroundColor.blueF()),
                 1.0f);

    m_gridRenderer.initializeGL(this);
    m_axisRenderer.initializeGL(this);

    for (auto& layer : m_layers)
        layer->initializeGL(this);
}

void GLViewport::fitCameraToLayers() {
    QVector3D globalMin, globalMax;
    bool      any = false;

    for (auto& layer : m_layers) {
        QVector3D lo, hi;
        if (layer->boundingBox(lo, hi)) {
            if (!any) {
                globalMin = lo;
                globalMax = hi;
                any       = true;
            } else {
                globalMin.setX(std::min(globalMin.x(), lo.x()));
                globalMin.setY(std::min(globalMin.y(), lo.y()));
                globalMin.setZ(std::min(globalMin.z(), lo.z()));
                globalMax.setX(std::max(globalMax.x(), hi.x()));
                globalMax.setY(std::max(globalMax.y(), hi.y()));
                globalMax.setZ(std::max(globalMax.z(), hi.z()));
            }
        }
    }

    if (!any)
        return;

    m_center         = (globalMin + globalMax) * 0.5f;
    const float diag = (globalMax - globalMin).length();
    m_distance       = diag > 0.0f ? diag * 0.75f : 10.0f;
    m_axisExtent     = diag > 0.0f ? diag * 0.5f : 10.0f;
    m_settings->grid()->setExtent(m_axisExtent);
}

void GLViewport::paintGL() {
    if (m_needsCameraFit) {
        fitCameraToLayers();
        m_needsCameraFit = false;
    }

    Common::RenderContext ctx = buildRenderContext();
    m_lastContext             = ctx;

    glClearColor(float(m_backgroundColor.redF()),
                 float(m_backgroundColor.greenF()),
                 float(m_backgroundColor.blueF()),
                 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gridRenderer.render(this, ctx, m_settings->grid());

    for (auto& layer : m_layers)
        if (layer->isVisible())
            layer->render(this, ctx);

    m_axisRenderer.render(this, ctx, m_settings->axis(), m_axisExtent);
}

// buildRenderContext(), paintEvent(), mouse*/wheelEvent(), resetCamera(),
// setCameraPreset(), forceFullRedraw(), setBackgroundColor() — без изменений
// от предыдущего сообщения.

} // namespace QSpace::Visualize::Views::View3D