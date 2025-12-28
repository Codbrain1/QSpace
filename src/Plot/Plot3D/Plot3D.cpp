#include "Plot/Plot3D/Plot3D.h"
#include <cstdint>
#include <gl/gl.h>
#include <qevent.h>
#include <qimage.h>
#include <qmatrix4x4.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qopenglext.h>
#include <qopenglshaderprogram.h>
#include <qopengltexture.h>
#include <qopenglwidget.h>
#include <qwidget.h>
#include <stdexcept>

namespace Plot3D {
Plot3D_Widget::Plot3D_Widget(QWidget* parent) : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setColorMap(ColorMap::Hubble);
    setParticleType(ParticleType::nonSPH);
}
Plot3D_Widget::~Plot3D_Widget() {
    makeCurrent();
    delete m_colorMapTex;
    doneCurrent();
}
//==========================================================
// initializeGL() - вызов один раз при запуске программы
//==========================================================
void Plot3D_Widget::initializeGL() {
    initializeOpenGLFunctions();          // включаем доступ к gl* функциям
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // задаем черный фон
    glEnable(GL_DEPTH_TEST);              // чтобы дальние частицы закрывались ближними
    glEnable(GL_BLEND);                   // включаем смешивание цветов
    glBlendFunc(GL_ONE, GL_ONE);          // additive blending — плотность складывается
    glEnable(GL_PROGRAM_POINT_SIZE);      // чтобы в шейдере можно было менять размер точки

    // загрузка шейдеров привязаны к exe
    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, ":/shaders/galaxy.vert");   // вершинный
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, ":/shaders/galaxy.frag"); // фрагментный
    m_program.link(); // компилируем и связываем шейдеры

    glGenVertexArrays(1, &m_vao); // создаём один VAO на всё приложение
    glBindVertexArray(m_vao);     // активируем его
    m_ssbo.create();              // создание буфера на GPU
}

//==========================================================
// resizeGL() - изменнение размера виджета
// loadColorMap() - загрузка цветовой карты
//==========================================================

void Plot3D_Widget::resizeGL(int w, int h) {
    m_proj.setToIdentity();                                    // сбрасываем матрицу
    m_proj.perspective(45.0f,                                  // угол обзора 45° — как нормальный объектив
                       static_cast<float>(w) / std::max(h, 1), // соотношение сторон
                       0.1f,                                   // ближняя плоскость
                       1000.0f                                 // дальняя плоскость
    );
}
void Plot3D_Widget::loadColorMap() {
    delete m_colorMapTex;
    QStringList names = {"viridis.png",
                         "magma.png",
                         "plasma.png",
                         "inferno.png",
                         "hubble.png",
                         "coolwarm.png",
                         "thermal.png",
                         "rainbow.png",
                         "stars.png"};

    QString path = ":/colormaps/" + names[static_cast<int>(cmap)];
    QImage img(path);
    if (img.isNull()) {
        if (img.isNull()) {
            qWarning() << "Failed to load colormap:" << path;
            return;
        }
    }
    QImage flipped = img.flipped(Qt::Vertical);
    m_colorMapTex = new QOpenGLTexture(flipped);
    m_colorMapTex->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    m_colorMapTex->setWrapMode(QOpenGLTexture::ClampToEdge);
}
//==========================================================
// paintGL() - основной метод отрисовки
//==========================================================
void Plot3D_Widget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // чистим экран и буфер глубины

    // вращение камеры
    QMatrix4x4 view;
    view.translate(0, 0, -m_distance);
    view.rotate(m_pitch, 1, 0, 0);
    view.rotate(m_yaw, 0, 1, 0);

    // активируем шейдеры
    m_program.bind();
    m_program.setUniformValue("mvp", m_proj * view);
    m_program.setUniformValue("globalScale", m_globalScale);
    m_program.setUniformValue("particleType", static_cast<int>(ptype));
    m_colorMapTex->bind(0);

    glBindVertexArray(m_vao);
    m_ssbo.bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo.bufferId()); // привязываем к слоту 0
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_data.size()));

    m_ssbo.release();
    m_program.release();
}
//==========================================================
// копирование данных из Ram в GPU
//==========================================================
void Plot3D_Widget::uploadData() {
    makeCurrent();                                                    // активируем контекст
    m_ssbo.bind();                                                    // выбор буфера
    m_ssbo.allocate(m_data.data(), m_data.size() * sizeof(Particle)); // алокация памяти GPU и копирование
    m_ssbo.release();                                                 // отпускание буфера
}

//==========================================================
// set методы для задания значений
//==========================================================
void Plot3D_Widget::setBrighness(float br) {
    if (br < 0)
        throw std::invalid_argument("brighness can't be less zero");
    m_brightness = br;
}
void Plot3D_Widget::setColorMap(ColorMap _cmap) {
    cmap = _cmap;
}
void Plot3D_Widget::setParticleType(ParticleType type) {
    ptype = type;
}
//==========================================================
// вращение мышкой
//==========================================================
void Plot3D_Widget::mousePressEvent(QMouseEvent* e) {
    m_lastPos = e->pos();
}
void Plot3D_Widget::mouseMoveEvent(QMouseEvent* e) {
    if (e->buttons() & Qt::LeftButton) {
        float dx = e->position().x() - m_lastPos.x();
        float dy = e->position().y() - m_lastPos.y();
        m_yaw += dx * 0.5f; // поворот влево вправво
        m_pitch
    }
}
} // namespace Plot3D