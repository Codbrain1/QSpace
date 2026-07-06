#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QScopedPointer>
#include <QVector3D>
#include <QVector>

#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <locale>

class ParticleGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
  public:
    explicit ParticleGLWidget(QWidget* parent = nullptr);
    ~ParticleGLWidget() override;

    // useLogScale: применить log10(|x|+eps) к скаляру перед нормализацией —
    // полезно для величин с большим динамическим диапазоном (плотность, энергия).
    void setParticles(const QVector<QVector3D>& positions,
                      const QVector<float>&     scalars,
                      bool                      useLogScale = false);

    // радиус сглаживания в мировых единицах для splat-прохода.
    // Если не задать явно — берётся авто-оценка по плотности частиц.
    void setSmoothingRadius(float r);

    // насколько сильно накопленная "масса" (перекрытие частиц в пикселе) влияет
    // на яркость — чем больше, тем быстрее насыщается яркость при малом перекрытии.
    void setMassBrightnessScale(float scale);

  signals:
    void firstFrameRendered(qint64 elapsedMs);
    void statsUpdated(double fps, double avgFrameMs);

  protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

  private:
    void buildShaders();
    void buildFullscreenQuad();
    void uploadParticleBuffers();
    void fitCameraToData();
    void ensureAccumFBO(int w, int h);
    void computePercentileRange(); // считает 1-й/99-й перцентиль вместо abs min/max

    QOpenGLShaderProgram m_splatProgram;   // проход 1: SPH-кернел -> сетка пикселей
    QOpenGLShaderProgram m_resolveProgram; // проход 2: усреднение + раскраска + яркость по массе

    QOpenGLBuffer            m_vboPos{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer            m_vboScalar{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vaoParticles;

    QOpenGLBuffer            m_vboQuad{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vaoQuad;

    // R = сумма (value*weight), G = сумма weight (аналог column density)
    QScopedPointer<QOpenGLFramebufferObject> m_accumFBO;

    QVector<QVector3D> m_positions;
    QVector<float>     m_scalars; // уже с применённым log, если useLogScale=true
    int                m_particleCount = 0;

    float m_scalarMin = 0.0f; // 1-й перцентиль
    float m_scalarMax = 1.0f; // 99-й перцентиль

    float m_smoothingRadiusWorld    = 0.05f;
    bool  m_smoothingRadiusExplicit = false;

    float m_massBrightnessScale = 2.5f;

    QVector3D m_center{0, 0, 0};
    float     m_distance = 10.0f;
    float     m_yaw      = 0.0f;
    float     m_pitch    = 0.0f;
    QPoint    m_lastMousePos;
    bool      m_dragging = false;

    QElapsedTimer m_loadToFirstFrameTimer;
    bool          m_firstFrameReported = false;

    QElapsedTimer m_statsTimer;
    int           m_framesSinceStats = 0;
    double        m_accumFrameMs     = 0.0;
};

ParticleGLWidget::ParticleGLWidget(QWidget* parent) : QOpenGLWidget(parent) {
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setSamples(4);
    setFormat(fmt);
}

ParticleGLWidget::~ParticleGLWidget() {
    makeCurrent();
    m_vboPos.destroy();
    m_vboScalar.destroy();
    m_vaoParticles.destroy();
    m_vboQuad.destroy();
    m_vaoQuad.destroy();
    m_accumFBO.reset();
    doneCurrent();
}

void ParticleGLWidget::setSmoothingRadius(float r) {
    m_smoothingRadiusWorld    = r;
    m_smoothingRadiusExplicit = true;
}

void ParticleGLWidget::setMassBrightnessScale(float scale) {
    m_massBrightnessScale = scale;
}

void ParticleGLWidget::computePercentileRange() {
    if (m_scalars.isEmpty()) {
        m_scalarMin = 0.0f;
        m_scalarMax = 1.0f;
        return;
    }

    QVector<float> sorted = m_scalars;
    std::sort(sorted.begin(), sorted.end());

    auto pct = [&](float p) -> float {
        const int idx =
            std::clamp(int(p * float(sorted.size() - 1)), 0, static_cast<int>(sorted.size() - 1));
        return sorted[idx];
    };

    m_scalarMin = pct(0.01f); // 1-й перцентиль вместо абсолютного минимума
    m_scalarMax = pct(0.99f); // 99-й перцентиль вместо абсолютного максимума

    if (m_scalarMax <= m_scalarMin)
        m_scalarMax = m_scalarMin + 1e-6f;
}

void ParticleGLWidget::setParticles(const QVector<QVector3D>& positions,
                                    const QVector<float>&     scalars,
                                    bool                      useLogScale) {
    m_positions     = positions;
    m_particleCount = positions.size();

    if (useLogScale) {
        m_scalars.resize(scalars.size());
        for (int i = 0; i < scalars.size(); ++i)
            m_scalars[i] = std::log10(std::abs(scalars[i]) + 1e-30f);
    } else {
        m_scalars = scalars;
    }

    computePercentileRange();
    fitCameraToData();

    m_firstFrameReported = false;
    m_loadToFirstFrameTimer.start();

    if (isValid()) {
        makeCurrent();
        uploadParticleBuffers();
        doneCurrent();
        update();
    }
}

void ParticleGLWidget::fitCameraToData() {
    if (m_positions.isEmpty())
        return;

    QVector3D minP = m_positions[0];
    QVector3D maxP = m_positions[0];
    for (const auto& p : m_positions) {
        minP.setX(std::min(minP.x(), p.x()));
        minP.setY(std::min(minP.y(), p.y()));
        minP.setZ(std::min(minP.z(), p.z()));
        maxP.setX(std::max(maxP.x(), p.x()));
        maxP.setY(std::max(maxP.y(), p.y()));
        maxP.setZ(std::max(maxP.z(), p.z()));
    }
    m_center         = (minP + maxP) * 0.5f;
    const float diag = (maxP - minP).length();
    m_distance       = (diag > 0.0f) ? diag * 1.5f : 10.0f;

    if (!m_smoothingRadiusExplicit && m_particleCount > 0 && diag > 0.0f) {
        const float n          = static_cast<float>(m_particleCount);
        m_smoothingRadiusWorld = (diag / std::cbrt(n)) * 1.2f;
    }
}

void ParticleGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glEnable(GL_PROGRAM_POINT_SIZE);

    buildShaders();
    buildFullscreenQuad();

    m_vaoParticles.create();
    m_vaoParticles.bind();
    m_vboPos.create();
    m_vboScalar.create();
    m_vaoParticles.release();

    if (!m_positions.isEmpty())
        uploadParticleBuffers();
}

void ParticleGLWidget::buildShaders() {
    // ============================= ПРОХОД 1: SPLAT =============================
    // SPH-кернел (кубический сплайн) вместо плоского диска: вклад частицы затухает
    // от центра к краю, как и должно быть в честной SPH-интерполяции W(r,h).
    static const char* splatVs = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in float aScalar;

        uniform mat4 uMVP;
        uniform float uPointScale;  // viewportHeight / (2*tan(fovY/2))
        uniform float uWorldRadius; // радиус сглаживания в мировых единицах

        out float vScalar;

        void main()
        {
            gl_Position = uMVP * vec4(aPos, 1.0);
            gl_PointSize = 2.0 * uPointScale * uWorldRadius / gl_Position.w;
            vScalar = aScalar;
        }
    )";

    static const char* splatFs = R"(
        #version 330 core
        in float vScalar;
        out vec4 FragColor;

        // кубический сплайн-кернел (2D-срез), r нормирован в [0,1]
        float kernelWeight(float r2)
        {
            float r = sqrt(r2);
            if (r >= 1.0) return 0.0;
            if (r < 0.5)  return 1.0 - 6.0 * r * r + 6.0 * r * r * r;
            float t = 1.0 - r;
            return 2.0 * t * t * t;
        }

        void main()
        {
            vec2 coord = gl_PointCoord * 2.0 - 1.0;
            float r2 = dot(coord, coord);
            if (r2 > 1.0)
                discard;

            float w = kernelWeight(r2);
            // R = сумма value*weight, G = сумма weight (аналог column density SPH)
            FragColor = vec4(vScalar * w, w, 0.0, 0.0);
        }
    )";

    m_splatProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, splatVs);
    m_splatProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, splatFs);
    if (!m_splatProgram.link())
        qWarning().noquote() << "Splat shader link error:" << m_splatProgram.log();

    // ======================= ПРОХОД 2: УСРЕДНЕНИЕ + ПАЛИТРА + ЯРКОСТЬ ПО МАССЕ
    // =======================
    static const char* resolveVs = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        out vec2 vUV;

        void main()
        {
            vUV = aPos * 0.5 + 0.5;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";

    static const char* resolveFs = R"(
        #version 330 core
        in vec2 vUV;
        out vec4 FragColor;

        uniform sampler2D uAccumTex;
        uniform float uScalarMin;
        uniform float uScalarMax;
        uniform float uMassBrightnessScale;

        vec3 heatmap(float t)
        {
            vec3 c0 = vec3(0.0, 0.0, 1.0);
            vec3 c1 = vec3(0.0, 1.0, 1.0);
            vec3 c2 = vec3(0.0, 1.0, 0.0);
            vec3 c3 = vec3(1.0, 1.0, 0.0);
            vec3 c4 = vec3(1.0, 0.0, 0.0);

            if (t < 0.25) return mix(c0, c1, t / 0.25);
            if (t < 0.5)  return mix(c1, c2, (t - 0.25) / 0.25);
            if (t < 0.75) return mix(c2, c3, (t - 0.5) / 0.25);
            return mix(c3, c4, (t - 0.75) / 0.25);
        }

        void main()
        {
            vec4 acc = texture(uAccumTex, vUV);
            float totalWeight = acc.g;
            if (totalWeight < 1e-5)
                discard; // ни одна частица не задела этот пиксель — фон

            // ---- усреднённое физическое поле (hue) ----
            float avgValue = acc.r / totalWeight;
            float t = clamp((avgValue - uScalarMin) / max(1e-6, uScalarMax - uScalarMin), 0.0, 1.0);
            vec3 baseColor = heatmap(t);

            // ---- яркость от накопленной "массы" перекрытия (аналог column density) ----
            // log сглаживает огромный разброс totalWeight между разреженными и плотными областями
            float massBrightness = clamp(log(1.0 + totalWeight) / uMassBrightnessScale, 0.0, 1.0);

            FragColor = vec4(baseColor * mix(0.15, 1.0, massBrightness), 1.0);
        }
    )";

    m_resolveProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, resolveVs);
    m_resolveProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, resolveFs);
    if (!m_resolveProgram.link())
        qWarning().noquote() << "Resolve shader link error:" << m_resolveProgram.log();
}

void ParticleGLWidget::buildFullscreenQuad() {
    static const float quad[] = {
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
    };

    m_vaoQuad.create();
    m_vaoQuad.bind();

    m_vboQuad.create();
    m_vboQuad.bind();
    m_vboQuad.allocate(quad, sizeof(quad));

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    m_vaoQuad.release();
}

void ParticleGLWidget::uploadParticleBuffers() {
    m_vaoParticles.bind();

    m_vboPos.bind();
    m_vboPos.allocate(m_positions.constData(),
                      m_positions.size() * static_cast<int>(sizeof(QVector3D)));
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), nullptr);
    glEnableVertexAttribArray(0);

    m_vboScalar.bind();
    if (!m_scalars.isEmpty()) {
        m_vboScalar.allocate(m_scalars.constData(),
                             m_scalars.size() * static_cast<int>(sizeof(float)));
    } else {
        QVector<float> zeros(m_positions.size(), 0.0f);
        m_vboScalar.allocate(zeros.constData(), zeros.size() * static_cast<int>(sizeof(float)));
    }
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), nullptr);
    glEnableVertexAttribArray(1);

    m_vaoParticles.release();
}

void ParticleGLWidget::ensureAccumFBO(int w, int h) {
    w = std::max(1, w);
    h = std::max(1, h);
    if (m_accumFBO && m_accumFBO->size() == QSize(w, h))
        return;

    QOpenGLFramebufferObjectFormat fmt;
    fmt.setInternalTextureFormat(GL_RGBA32F);
    fmt.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    m_accumFBO.reset(new QOpenGLFramebufferObject(w, h, fmt));
}

void ParticleGLWidget::resizeGL(int /*w*/, int /*h*/) {
    // размер аккумулирующего FBO пересоздаётся лениво в paintGL через ensureAccumFBO
}

void ParticleGLWidget::paintGL() {
    QElapsedTimer frameClock;
    frameClock.start();

    ensureAccumFBO(width(), height());

    QMatrix4x4  proj;
    const float fovYDeg = 45.0f;
    float       aspect  = width() > 0 ? float(width()) / float(std::max(1, height())) : 1.0f;
    proj.perspective(fovYDeg, aspect, 0.01f * m_distance, 100.0f * m_distance);

    QVector3D eye(m_center.x() + m_distance * std::cos(m_pitch) * std::sin(m_yaw),
                  m_center.y() + m_distance * std::sin(m_pitch),
                  m_center.z() + m_distance * std::cos(m_pitch) * std::cos(m_yaw));

    QMatrix4x4 view;
    view.lookAt(eye, m_center, QVector3D(0, 1, 0));
    QMatrix4x4 mvp = proj * view;

    const float fovYRad    = qDegreesToRadians(fovYDeg);
    const float pointScale = float(height()) / (2.0f * std::tan(fovYRad * 0.5f));

    if (m_particleCount > 0) {
        // ================= ПРОХОД 1: splat SPH-кернелом в аккумулирующую текстуру
        // =================
        m_accumFBO->bind();
        glViewport(0, 0, m_accumFBO->width(), m_accumFBO->height());
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

        m_splatProgram.bind();
        m_splatProgram.setUniformValue("uMVP", mvp);
        m_splatProgram.setUniformValue("uPointScale", pointScale);
        m_splatProgram.setUniformValue("uWorldRadius", m_smoothingRadiusWorld);

        m_vaoParticles.bind();
        glDrawArrays(GL_POINTS, 0, m_particleCount);
        m_vaoParticles.release();
        m_splatProgram.release();

        glDisable(GL_BLEND);
        m_accumFBO->release();
    }

    // ================= ПРОХОД 2: усреднение + раскраска + яркость по массе =================
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0, 0, width(), height());
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_particleCount > 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_accumFBO->texture());

        m_resolveProgram.bind();
        m_resolveProgram.setUniformValue("uAccumTex", 0);
        m_resolveProgram.setUniformValue("uScalarMin", m_scalarMin);
        m_resolveProgram.setUniformValue("uScalarMax", m_scalarMax);
        m_resolveProgram.setUniformValue("uMassBrightnessScale", m_massBrightnessScale);

        m_vaoQuad.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        m_vaoQuad.release();
        m_resolveProgram.release();

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // ========================= ЗАМЕРЫ ПРОИЗВОДИТЕЛЬНОСТИ =========================
    glFinish();

    const double frameMs = frameClock.nsecsElapsed() / 1e6;

    if (!m_firstFrameReported && m_particleCount > 0) {
        m_firstFrameReported = true;
        emit firstFrameRendered(m_loadToFirstFrameTimer.elapsed());
    }

    m_accumFrameMs += frameMs;
    ++m_framesSinceStats;
    if (!m_statsTimer.isValid())
        m_statsTimer.start();
    if (m_statsTimer.elapsed() >= 1000) {
        const double avgFrameMs = m_accumFrameMs / std::max(1, m_framesSinceStats);
        const double fps        = avgFrameMs > 0.0 ? 1000.0 / avgFrameMs : 0.0;
        emit         statsUpdated(fps, avgFrameMs);
        m_framesSinceStats = 0;
        m_accumFrameMs     = 0.0;
        m_statsTimer.restart();
    }

    update();
}

void ParticleGLWidget::mousePressEvent(QMouseEvent* event) {
    m_dragging     = true;
    m_lastMousePos = event->pos();
}

void ParticleGLWidget::mouseMoveEvent(QMouseEvent* event) {
    if (!m_dragging)
        return;
    QPoint delta   = event->pos() - m_lastMousePos;
    m_lastMousePos = event->pos();

    m_yaw += delta.x() * 0.01f;
    m_pitch += delta.y() * 0.01f;
    m_pitch = std::clamp(m_pitch, -1.5f, 1.5f);
}

void ParticleGLWidget::wheelEvent(QWheelEvent* event) {
    float delta = event->angleDelta().y() / 120.0f;
    m_distance *= std::pow(0.9f, delta);
    m_distance = std::max(m_distance, 0.001f);
}

#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Structures/FileSchemeStructures.h"
#include "IO/ReaderFactory.h"
#include "IO/SchemeFactory.h"


#include <QApplication>
#include <QElapsedTimer>
#include <QVector3D>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    QElapsedTimer startupTimer;
    startupTimer.start();

    QApplication app(argc, argv);

    const QString dataPath = "D:/NIR/NIR_6_semestr/QSpace/Sandbox/TEST_DATA/TF250_v2/DM_    0.bin";

    auto reader = QSpace::IO::createReader(QSpace::IO::FileFormat::BIN);
    if (!reader) {
        qCritical() << "Failed to create reader";
        return 1;
    }
    reader->setPolicy(QSpace::IO::FilePolicy::ForceStandart);

    auto scheme =
        QSpace::IO::SchemeFactory::createScheme_v2(QSpace::Visualize::EntityType::DarkMatter,
                                                   QSpace::IO::FileFormat::BIN);

    QElapsedTimer readTimer;
    readTimer.start();
    auto readResult = reader->read(dataPath, scheme);
    qDebug().noquote() << QString("Чтение файла заняло %1 мс").arg(readTimer.elapsed());

    if (!readResult.isSuccess()) {
        qCritical() << "Failed to read data";
        return 1;
    }

    vtkPolyData* polyData = vtkPolyData::SafeDownCast(readResult.data);
    if (!polyData || !polyData->GetPoints() || polyData->GetPoints()->GetNumberOfPoints() <= 0) {
        qCritical() << "Invalid polyData or empty points";
        return 1;
    }

    vtkPoints*      points = polyData->GetPoints();
    const vtkIdType n      = points->GetNumberOfPoints();

    QVector<QVector3D> positions;
    positions.reserve(static_cast<int>(n));
    for (vtkIdType i = 0; i < n; ++i) {
        double p[3];
        points->GetPoint(i, p);
        positions.append(QVector3D(static_cast<float>(p[0]),
                                   static_cast<float>(p[1]),
                                   static_cast<float>(p[2])));
    }

    QVector<float> scalars;
    vtkDataArray*  densityArray = polyData->GetPointData()->GetArray("Mass");
    if (densityArray && densityArray->GetNumberOfTuples() == n) {
        scalars.reserve(static_cast<int>(n));
        for (vtkIdType i = 0; i < n; ++i)
            scalars.append(static_cast<float>(densityArray->GetTuple1(i)));
    }

    ParticleGLWidget widget;
    widget.resize(1024, 768);
    widget.setWindowTitle("SPH Particles (Qt OpenGL)");

    // Для тёмной материи структуры (гало, филаменты) видны прежде всего в накоплении
    // (перекрытии частиц), а не в разбросе самой массы одной частицы — поэтому здесь
    // важнее настройка uMassBrightnessScale, чем логарифмирование Mass.
    // Если поле Mass у DM почти константно по частицам — можно передать useLogScale=false
    // и полагаться в основном на яркость по накоплению.
    widget.setMassBrightnessScale(4.0f);

    QObject::connect(&widget, &ParticleGLWidget::firstFrameRendered, [&](qint64 ms) {
        qDebug().noquote() << QString("Время до первого отображения частиц: %1 мс").arg(ms);
        qDebug().noquote() << QString("Полное время от старта программы до первого кадра: %1 мс")
                                  .arg(startupTimer.elapsed());
    });

    QObject::connect(&widget,
                     &ParticleGLWidget::statsUpdated,
                     [&widget](double fps, double avgFrameMs) {
                         widget.setWindowTitle(
                             QString("SPH Particles (Qt OpenGL) — %1 FPS (%2 мс/кадр)")
                                 .arg(fps, 0, 'f', 1)
                                 .arg(avgFrameMs, 0, 'f', 2));
                     });

    widget.show();
    widget.setParticles(positions, scalars, /*useLogScale=*/false);

    return app.exec();
}

#include "main.moc"
