// Visualize/Layers/ParticleRendererLayer.cpp
#include "ParticleRendererLayer.h"
#include "Common/Structures/RenderContext.h"
#include "Visualize/ColorMapManager/ColorMapTexture.h"
#include <algorithm>

namespace QSpace::Visualize::Layers {

void ParticleRendererLayer::buildShader() {
    static const char* vs = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in float aScalar;
        uniform mat4 uMVP;
        uniform float uPointSize;
        uniform float uMinSize;
        uniform float uMaxSize;
        uniform bool uSizeByField;
        uniform float uRangeMin;
        uniform float uRangeMax;
        uniform bool uUseLogScale;
        out float vNorm;
        void main() {
            gl_Position = uMVP * vec4(aPos, 1.0);
            float value = uUseLogScale ? log(max(abs(aScalar), 1e-30)) / log(10.0) : aScalar;
            float t = clamp((value - uRangeMin) / max(1e-6, uRangeMax - uRangeMin), 0.0, 1.0);
            gl_PointSize = uSizeByField ? mix(uMinSize, uMaxSize, t) : uPointSize;
            vNorm = t;
        }
    )";
    static const char* fs = R"(
        #version 330 core
        in float vNorm;
        out vec4 FragColor;
        uniform sampler1D uColorMap;
        uniform bool uShadeAsSphere;
        uniform float uOpacity;
        void main() {
            vec2 coord = gl_PointCoord * 2.0 - 1.0;
            float r2 = dot(coord, coord);
            if (r2 > 1.0) discard;
            vec3 color = texture(uColorMap, vNorm).rgb;
            if (uShadeAsSphere) {
                float shade = sqrt(1.0 - r2);
                color *= (0.5 + 0.5 * shade);
            }
            FragColor = vec4(color, uOpacity);
        }
    )";
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
    if (!m_program.link())
        qWarning().noquote() << "ParticleRendererLayer shader error:" << m_program.log();
}

void ParticleRendererLayer::initializeGL(QOpenGLFunctions_3_3_Core* gl) {
    Q_UNUSED(gl);
    buildShader();
    m_vao.create();
    m_vao.bind();
    m_vboPos.create();
    m_vboScalar.create();
    m_vao.release();
}

void ParticleRendererLayer::uploadBuffersIfDirty(QOpenGLFunctions_3_3_Core* gl) {
    if (!m_dirty)
        return;
    auto node = m_dataNode.lock();
    if (!node)
        return;

    // ПРЕДПОЛОЖЕНИЕ: адаптер DataNode -> позиции/скаляр по имени colorByField.
    QVector<QVector3D> positions;
    QVector<float>     scalars; // = node->scalarField(m_settings->colorByField());
    m_particleCount = positions.size();

    m_vao.bind();
    m_vboPos.bind();
    m_vboPos.allocate(positions.constData(), positions.size() * int(sizeof(QVector3D)));
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
    gl->glEnableVertexAttribArray(0);

    m_vboScalar.bind();
    if (!scalars.isEmpty())
        m_vboScalar.allocate(scalars.constData(), scalars.size() * int(sizeof(float)));
    else {
        QVector<float> zeros(positions.size(), 0.0f);
        m_vboScalar.allocate(zeros.constData(), zeros.size() * int(sizeof(float)));
    }
    gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr);
    gl->glEnableVertexAttribArray(1);
    m_vao.release();

    m_dirty = false;

    if (!scalars.isEmpty())
        autoCalibrateRangeIfNeeded();
}

void ParticleRendererLayer::autoCalibrateRangeIfNeeded() {
    if (!m_settings || !m_settings->autoRange())
        return;
    auto node = m_dataNode.lock();
    if (!node)
        return;

    // ПРЕДПОЛОЖЕНИЕ: пересчёт перцентилей поля colorByField на CPU при загрузке.
    // Заполняет базовые rangeMin/rangeMax (используются всеми тремя рендерерами одинаково).
    QVector<float> scalars; // = node->scalarField(m_settings->colorByField());
    if (scalars.isEmpty())
        return;

    QVector<float> sorted = scalars;
    if (m_settings->useLogScale())
        for (auto& v : sorted)
            v = std::log10(std::max(std::abs(v), 1e-30f));
    std::sort(sorted.begin(), sorted.end());

    auto pct = [&](float p) {
        int idx = std::clamp(int(p * (sorted.size() - 1)), 0, static_cast<int>(sorted.size() - 1));
        return sorted[idx];
    };
    m_settings->setBaseRangeMin(pct(0.01f));
    m_settings->setBaseRangeMax(pct(0.99f));
    m_settings->setRangeMin(pct(0.01f));
    m_settings->setRangeMax(pct(0.99f));
}

void ParticleRendererLayer::render(QOpenGLFunctions_3_3_Core*      gl,
                                   const Visualize::RenderContext& ctx) {
    if (!m_settings || m_particleCount == 0)
        return;
    uploadBuffersIfDirty(gl);

    gl->glEnable(GL_BLEND);
    if (static_cast<LayerSettings::BlendMode>(m_settings->blendMode()) ==
        LayerSettings::BlendMode::Additive)
        gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    else
        gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const GLuint colorMapTex = ColorMapTexture::getOrCreate(gl, m_settings->colorMapId());
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_1D, colorMapTex);

    m_program.bind();
    m_program.setUniformValue("uMVP", ctx.mvp);
    m_program.setUniformValue("uPointSize", m_settings->pointSizePx());
    m_program.setUniformValue("uMinSize", m_settings->minPointSizePx());
    m_program.setUniformValue("uMaxSize", m_settings->maxPointSizePx());
    m_program.setUniformValue("uSizeByField", m_settings->sizeByField());
    m_program.setUniformValue("uRangeMin", float(m_settings->rangeMin()));
    m_program.setUniformValue("uRangeMax", float(m_settings->rangeMax()));
    m_program.setUniformValue("uUseLogScale", m_settings->useLogScale());
    m_program.setUniformValue("uShadeAsSphere", m_settings->shadeAsSphere());
    m_program.setUniformValue("uOpacity", float(m_settings->opacity()));
    m_program.setUniformValue("uColorMap", 0);

    m_vao.bind();
    gl->glDrawArrays(GL_POINTS, 0, m_particleCount);
    m_vao.release();
    m_program.release();

    gl->glBindTexture(GL_TEXTURE_1D, 0);
    gl->glDisable(GL_BLEND);
}

void ParticleRendererLayer::releaseGL(QOpenGLFunctions_3_3_Core*) {
    m_vboPos.destroy();
    m_vboScalar.destroy();
    m_vao.destroy();
}

bool ParticleRendererLayer::boundingBox(QVector3D&, QVector3D&) const {
    return false;
}

} // namespace QSpace::Visualize::Layers