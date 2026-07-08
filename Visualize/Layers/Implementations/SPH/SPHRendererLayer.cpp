#include "SPHRendererLayer.h"
#include "Visualize/ColorMapManager/ColorMapTexture.h"
#include "Visualize/Layers/vtkAdapter.h"
#include <algorithm>
#include <cmath>

namespace QSpace::Visualize::Layers {

void SPHRendererLayer::buildShaders() {
    static const char* splatVs = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in float aMass;
        uniform mat4 uMVP;
        uniform float uPointScale;
        uniform float uWorldRadius;
        uniform float uKernelNorm;
        out float vMass;
        out float vKernelNorm;
        void main() {
            gl_Position = uMVP * vec4(aPos, 1.0);
            gl_PointSize = 2.0 * uPointScale * uWorldRadius; // была пропущена '*' — исправлено
            vMass = aMass;
            vKernelNorm = uKernelNorm;
        }
    )";
    static const char* splatFs = R"(
        #version 330 core
        in float vMass;
        in float vKernelNorm;
        out vec4 FragColor;
        uniform int uKernelType;
        float cubicSpline(float q) {
            if (q >= 1.0) return 0.0;
            if (q < 0.5) return 1.0 - 6.0*q*q + 6.0*q*q*q;
            float t = 1.0 - q; return 2.0*t*t*t;
        }
        float gaussianKernel(float q) { return q >= 1.0 ? 0.0 : exp(-4.0*q*q); }
        float wendlandKernel(float q) {
            if (q >= 1.0) return 0.0;
            float t = 1.0 - q; return t*t*t*t * (1.0 + 4.0*q);
        }
        void main() {
            vec2 coord = gl_PointCoord * 2.0 - 1.0;
            float q2 = dot(coord, coord);
            if (q2 > 1.0) discard;
            float q = sqrt(q2);
            float shape = uKernelType == 1 ? gaussianKernel(q)
                        : uKernelType == 2 ? wendlandKernel(q)
                        : cubicSpline(q);
            FragColor = vec4(vMass * vKernelNorm * shape, 0.0, 0.0, 0.0);
        }
    )";
    m_splatProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, splatVs);
    m_splatProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, splatFs);
    if (!m_splatProgram.link())
        qWarning().noquote() << "SPH splat shader error:" << m_splatProgram.log();

    static const char* resolveVs = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        out vec2 vUV;
        void main() { vUV = aPos*0.5+0.5; gl_Position = vec4(aPos,0.0,1.0); }
    )";
    static const char* resolveFs = R"(
        #version 330 core
        in vec2 vUV;
        out vec4 FragColor;
        uniform sampler2D uAccumTex;
        uniform sampler1D uColorMap;
        uniform float uRangeMin;
        uniform float uRangeMax;
        uniform bool  uUseLogScale;
        uniform float uGamma;
        uniform float uOpacity;
        void main() {
            float density = texture(uAccumTex, vUV).r;
            if (density < 1e-12) discard;
            float value = uUseLogScale ? log(density) / log(10.0) : density;
            float t = clamp((value - uRangeMin) / max(1e-6, uRangeMax - uRangeMin), 0.0, 1.0);
            t = pow(t, 1.0 / max(0.01, uGamma));
            FragColor = vec4(texture(uColorMap, t).rgb, uOpacity);
        }
    )";
    m_resolveProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, resolveVs);
    m_resolveProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, resolveFs);
    if (!m_resolveProgram.link())
        qWarning().noquote() << "SPH resolve shader error:" << m_resolveProgram.log();
}

void SPHRendererLayer::initializeGL(QOpenGLFunctions_3_3_Core* gl) {
    buildShaders();

    static const float quad[] = {-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1};
    m_vaoQuad.create();
    m_vaoQuad.bind();
    m_vboQuad.create();
    m_vboQuad.bind();
    m_vboQuad.allocate(quad, sizeof(quad));
    gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    gl->glEnableVertexAttribArray(0);
    m_vaoQuad.release();

    m_vaoParticles.create();
    m_vaoParticles.bind();
    m_vboPos.create();
    m_vboMass.create();
    m_vaoParticles.release();
}

void SPHRendererLayer::uploadBuffersIfDirty(QOpenGLFunctions_3_3_Core* gl) {
    if (!m_dirty)
        return;
    auto node = m_dataNode.lock();
    if (!node)
        return;

    QVector<QVector3D> positions = Visualize::vtkAdapter::extractPositions(node);

    // "mass" здесь — это то самое поле, которое проецируется и накапливается
    // как поверхностная плотность (colorByField). Если поле не задано — считаем
    // все частицы равновесными (weight=1), и результат — числовая плотность частиц.
    QVector<float> masses;
    if (m_settings && !m_settings->colorByField().isEmpty())
        masses = Visualize::vtkAdapter::extractScalarField(node, m_settings->colorByField());

    m_particleCount = positions.size();
    if (m_particleCount == 0) {
        qWarning() << "SPHRendererLayer: DataNode contains no points";
        m_dirty     = false;
        m_hasBounds = false;
        return;
    }

    m_boundsMin = m_boundsMax = positions[0];
    for (const auto& p : positions) {
        m_boundsMin.setX(std::min(m_boundsMin.x(), p.x()));
        m_boundsMin.setY(std::min(m_boundsMin.y(), p.y()));
        m_boundsMin.setZ(std::min(m_boundsMin.z(), p.z()));
        m_boundsMax.setX(std::max(m_boundsMax.x(), p.x()));
        m_boundsMax.setY(std::max(m_boundsMax.y(), p.y()));
        m_boundsMax.setZ(std::max(m_boundsMax.z(), p.z()));
    }
    m_hasBounds = true;

    m_vaoParticles.bind();
    m_vboPos.bind();
    m_vboPos.allocate(positions.constData(), positions.size() * int(sizeof(QVector3D)));
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
    gl->glEnableVertexAttribArray(0);

    m_vboMass.bind();
    if (!masses.isEmpty()) {
        m_vboMass.allocate(masses.constData(), masses.size() * int(sizeof(float)));
    } else {
        QVector<float> ones(positions.size(), 1.0f);
        m_vboMass.allocate(ones.constData(), ones.size() * int(sizeof(float)));
    }
    gl->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr);
    gl->glEnableVertexAttribArray(1);
    m_vaoParticles.release();

    m_dirty            = false;
    m_needsCalibration = m_settings && m_settings->autoRange();
}

void SPHRendererLayer::ensureAccumFBO(const QSize& size) {
    const float scale = m_settings ? m_settings->accumResolutionScale() : 1.0f;
    const QSize scaled(std::max(1, int(size.width() * scale)),
                       std::max(1, int(size.height() * scale)));
    if (m_accumFBO && m_accumFBO->size() == scaled)
        return;

    QOpenGLFramebufferObjectFormat fmt;
    fmt.setInternalTextureFormat(GL_RGBA32F);
    fmt.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    m_accumFBO.reset(new QOpenGLFramebufferObject(scaled, fmt));
}

void SPHRendererLayer::calibrateIfNeeded(QOpenGLFunctions_3_3_Core* gl) {
    if (!m_needsCalibration || !m_accumFBO || !m_settings)
        return;

    const int      w = m_accumFBO->width(), h = m_accumFBO->height();
    QVector<float> pixels(w * h * 4);
    m_accumFBO->bind();
    gl->glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, pixels.data());
    m_accumFBO->release();

    QVector<float> logValues;
    logValues.reserve(w * h / 4);
    for (int i = 0; i < w * h; ++i) {
        const float d = pixels[i * 4];
        if (d > 1e-12f)
            logValues.append(m_settings->useLogScale() ? std::log10(d) : d);
    }
    if (!logValues.isEmpty()) {
        std::sort(logValues.begin(), logValues.end());
        auto pct = [&](float p) {
            int idx = std::clamp(int(p * (logValues.size() - 1)),
                                 0,
                                 static_cast<int>(logValues.size() - 1));
            return logValues[idx];
        };
        m_settings->setRangeMin(pct(0.02f));
        m_settings->setRangeMax(pct(0.995f));
        m_settings->setBaseRangeMin(pct(0.02f));
        m_settings->setBaseRangeMax(pct(0.995f));
    }
    m_needsCalibration = false;
}

void SPHRendererLayer::render(QOpenGLFunctions_3_3_Core* gl, const Visualize::RenderContext& ctx) {
    if (!m_settings)
        return;
    uploadBuffersIfDirty(gl);

    if (m_particleCount == 0)
        return;
    ensureAccumFBO(ctx.viewportPx);

    const float h          = m_settings->smoothingRadius();
    const float kernelNorm = 40.0f / (7.0f * float(M_PI) * h * h);

    m_accumFBO->bind();
    gl->glViewport(0, 0, m_accumFBO->width(), m_accumFBO->height());
    gl->glClearColor(0, 0, 0, 0);
    gl->glClear(GL_COLOR_BUFFER_BIT);
    gl->glDisable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_ONE, GL_ONE);

    m_splatProgram.bind();
    m_splatProgram.setUniformValue("uMVP", ctx.mvp);
    m_splatProgram.setUniformValue("uPointScale", ctx.pixelsPerWorldUnit);
    m_splatProgram.setUniformValue("uWorldRadius", h);
    m_splatProgram.setUniformValue("uKernelNorm", kernelNorm);
    m_splatProgram.setUniformValue("uKernelType", m_settings->kernelType());

    m_vaoParticles.bind();
    gl->glDrawArrays(GL_POINTS, 0, m_particleCount);
    m_vaoParticles.release();
    m_splatProgram.release();
    gl->glDisable(GL_BLEND);
    m_accumFBO->release();

    calibrateIfNeeded(gl);

    gl->glViewport(0, 0, ctx.viewportPx.width(), ctx.viewportPx.height());
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, m_accumFBO->texture());
    gl->glActiveTexture(GL_TEXTURE1);
    gl->glBindTexture(GL_TEXTURE_1D, ColorMapTexture::getOrCreate(gl, m_settings->colorMapId()));

    m_resolveProgram.bind();
    m_resolveProgram.setUniformValue("uAccumTex", 0);
    m_resolveProgram.setUniformValue("uColorMap", 1);
    m_resolveProgram.setUniformValue("uRangeMin", float(m_settings->rangeMin()));
    m_resolveProgram.setUniformValue("uRangeMax", float(m_settings->rangeMax()));
    m_resolveProgram.setUniformValue("uUseLogScale", m_settings->useLogScale());
    m_resolveProgram.setUniformValue("uGamma", m_settings->densityGamma());
    m_resolveProgram.setUniformValue("uOpacity", float(m_settings->opacity()));

    m_vaoQuad.bind();
    gl->glDrawArrays(GL_TRIANGLES, 0, 6);
    m_vaoQuad.release();
    m_resolveProgram.release();
    gl->glBindTexture(GL_TEXTURE_2D, 0);
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_1D, 0);
    gl->glDisable(GL_BLEND);
}

void SPHRendererLayer::releaseGL(QOpenGLFunctions_3_3_Core*) {
    m_vboPos.destroy();
    m_vboMass.destroy();
    m_vaoParticles.destroy();
    m_vboQuad.destroy();
    m_vaoQuad.destroy();
    m_accumFBO.reset();
}

bool SPHRendererLayer::boundingBox(QVector3D& outMin, QVector3D& outMax) const {
    if (!m_hasBounds)
        return false;
    outMin = m_boundsMin;
    outMax = m_boundsMax;
    return true;
}

} // namespace QSpace::Visualize::Layers