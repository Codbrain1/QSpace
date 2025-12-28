#pragma once
#include <QOpenGLWidget>
#include <gl/gl.h>
#include <qevent.h>
#include <qmatrix4x4.h>
#include <qopenglcontext.h>
#include <qopenglfunctions.h>
#include <qopenglshaderprogram.h>
#include <qopenglwidget.h>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLShaderProgram>
#include <qpoint.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <vector>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
namespace Plot3D {
enum ColorMap { Viridis, Magma, Plasma, Inferno, Hubble, CoolWarm, Thermal, Rainbow, Stars };
// enum Color {};
enum ParticleType { SPH, nonSPH };
class Plot3D_Widget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT
  protected:
    // вызываются автоматически
    void initializeGL() override;         // задает начальные параметры настройки графика (при запуске программы)
    void resizeGL(int w, int h) override; // определяет действия при изменении размера окна
    void paintGL() override;              // основной метод рендера

    void mousePressEvent(QMouseEvent* e) override; // нажатие кнопки мыши
    void mouseMoveEvent(QMouseEvent* e) override;  // перемещение мыши
    void wheelEvent(QWheelEvent* e) override;      // вращение колесиком

  private:
    struct Particle {     // агрегация данных частицы
        float pos[3];     // позиция частицы
        float value;      // значение для отрисовки
        float hsml = 1.0; // сглаживание
        float brighness_factor = 1.0;
    };

    QOpenGLShaderProgram m_program; // хранит скомпилированные шейдеры
    QOpenGLBuffer m_ssbo;           // буфер на видеокарте
    GLuint m_vao = 0;               // конфигурация того как буферы привязаны к вершинам шейдера
    std::vector<Particle> m_data;   // копия данных на CPU для перезагрузки на GPU при отрисовке
    QOpenGLTexture* m_colorMapTex = nullptr;

    // камера
    QMatrix4x4 m_proj;        // матрица проекции
    float m_yaw = -35.0f;     // угол поворота
    float m_pitch = -25.0f;   // угол наклона
    float m_distance = 80.0f; // расстояние от камеры до галактики
    QPoint m_lastPos;         // последняя позиция мыши

    // параметры рендера
    float m_globalScale = 30.0f; // общий масштаб (автоматически)
    float m_brightness = 1.4f;   // общая яркость
    ColorMap cmap;               // цветовая карта
    ParticleType ptype;          // тип частиц
    void uploadData();           // копирование данных из RAM в GPU
    void loadColorMap();

  public:
    explicit Plot3D_Widget(QWidget* parent = nullptr);
    ~Plot3D_Widget();
    // передает данные для рисования
    void plot(const std::vector<double> x,
              const std::vector<double> y,
              const std::vector<double> z,
              const std::vector<double> value,
              ParticleType type = ParticleType::nonSPH);
    void empty_plot();                // очистка фигуры
    void setColorMap(ColorMap _cmap); // установка схемы раскраски точек
    void setParticleType(ParticleType type);
    void setBrighness(float br);
};
} // namespace Plot3D