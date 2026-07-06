
#include "AxisRenderer.h"

namespace QSpace::Visualize::Views::View3D {

AxisRenderer::AxisRenderer(QObject* parent) : QObject(parent) {
}

void AxisRenderer::buildShader() {
    // вершинный цвет передаётся как атрибут — три оси разного цвета в одном draw call
    static const char* vs = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;
        uniform mat4 uMVP;
        out vec3 vColor;
        void main() {
            gl_Position = uMVP * vec4(aPos, 1.0);
            vColor = aColor;
        }
    )";
    static const char* fs = R"(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main() { FragColor = vec4(vColor, 1.0); }
    )";
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
    if (!m_program.link())
        qWarning().noquote() << "AxisRenderer shader link error:" << m_program.log();
}

void AxisRenderer::initializeGL(QOpenGLFunctions_3_3_Core* gl) {
    Q_UNUSED(gl);
    buildShader();
    m_vao.create();
    m_vao.bind();
    m_vbo.create();
    m_vao.release();
    m_glInitialized = true;
}

void AxisRenderer::releaseGL(QOpenGLFunctions_3_3_Core* gl) {
    Q_UNUSED(gl);
    m_vbo.destroy();
    m_vao.destroy();
    m_glInitialized = false;
}

void AxisRenderer::rebuildGeometry(QOpenGLFunctions_3_3_Core* gl, float extent) {
    // pos(3) + color(3) на вершину; цвета берутся из settings в render(),
    // поэтому геометрию перестраиваем только при смене extent, а цвет — uniform-подобно
    // через сам атрибут (проще перестроить, чем городить отдельный uniform на ось)
    struct V {
        float x, y, z, r, g, b;
    };

    // цвета здесь placeholder — реальные берутся из AxisSettings в render() через rebuild
    QVector<V> verts = {
        {0, 0, 0, 1, 0, 0},
        {extent, 0, 0, 1, 0, 0}, // X
        {0, 0, 0, 0, 1, 0},
        {0, extent, 0, 0, 1, 0}, // Y
        {0, 0, 0, 0, 0, 1},
        {0, 0, extent, 0, 0, 1}, // Z
    };

    m_vertexCount = verts.size();

    m_vao.bind();
    m_vbo.bind();
    m_vbo.allocate(verts.constData(), verts.size() * int(sizeof(V)));
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), nullptr);
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(1,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(V),
                              reinterpret_cast<void*>(3 * sizeof(float)));
    gl->glEnableVertexAttribArray(1);
    m_vao.release();

    m_cachedExtent = extent;
}

void AxisRenderer::render(QOpenGLFunctions_3_3_Core*      gl,
                          const Visualize::RenderContext& ctx,
                          AxisSettings*                   settings,
                          float                           extent) {
    if (!settings->visible() || !m_glInitialized)
        return;

    if (!qFuzzyCompare(m_cachedExtent, extent)) {
        rebuildGeometry(gl, extent);

        // после rebuild подставляем актуальные цвета из settings —
        // проще перезалить буфер с нужными цветами, чем держать три отдельных draw call
        struct V {
            float x, y, z, r, g, b;
        };

        const QColor cx = settings->colorX(), cy = settings->colorY(), cz = settings->colorZ();
        QVector<V>   verts = {
            {0, 0, 0, float(cx.redF()), float(cx.greenF()), float(cx.blueF())},
            {extent, 0, 0, float(cx.redF()), float(cx.greenF()), float(cx.blueF())},
            {0, 0, 0, float(cy.redF()), float(cy.greenF()), float(cy.blueF())},
            {0, extent, 0, float(cy.redF()), float(cy.greenF()), float(cy.blueF())},
            {0, 0, 0, float(cz.redF()), float(cz.greenF()), float(cz.blueF())},
            {0, 0, extent, float(cz.redF()), float(cz.greenF()), float(cz.blueF())},
        };
        m_vbo.bind();
        m_vbo.write(0, verts.constData(), verts.size() * int(sizeof(V)));
    }

    m_program.bind();
    m_program.setUniformValue("uMVP", ctx.mvp);

    gl->glLineWidth(2.0f); // толще, чем сетка — оси должны визуально выделяться
    m_vao.bind();
    gl->glDrawArrays(GL_LINES, 0, m_vertexCount);
    m_vao.release();
    gl->glLineWidth(1.0f);

    m_program.release();
}

QVector<AxisTick> AxisRenderer::buildTicks(AxisSettings* settings, float extent) const {
    QVector<AxisTick> ticks;
    if (!settings->showTicks())
        return ticks;

    const int   n    = std::max(1, settings->tickCount());
    const float step = extent / float(n);

    for (int i = 1; i <= n; ++i) {
        const float   v     = i * step;
        const QString label = QString::number(v, 'g', 3) + " " + settings->unitLabel();

        ticks.append({QVector3D(v, 0, 0), label, settings->colorX()});
        ticks.append({QVector3D(0, v, 0), label, settings->colorY()});
        ticks.append({QVector3D(0, 0, v), label, settings->colorZ()});
    }
    return ticks;
}

} // namespace QSpace::Visualize::Views::View3D