#pragma once
#include "Common/Interfaces/IRenderLayer.h"
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QScopedPointer>
#include "BinningPointsLayerSettings.h" // переименовано из BiningPointsLayerSettings — см. примечание

namespace QSpace::Visualize::Layers {

class BinningRendererLayer : public IOpenGLRenderLayer {
  public:
    void update() override {
        m_dirty = true;
    }

    void setData(std::weak_ptr<Core::DataNode> node) override {
        m_dataNode = node;
        m_dirty    = true;
    }

    void setSettings(std::shared_ptr<LayerSettings> settings) override {
        m_settings = std::dynamic_pointer_cast<BinningPointsLayerSettings>(settings);
    }

    std::shared_ptr<LayerSettings> getSettings() const override {
        return m_settings;
    }

    void setVisible(bool visible) override {
        m_visible = visible;
    }

    bool isVisible() const override {
        return m_visible;
    }

    std::shared_ptr<Visualize::IRenderLayer> clone() const override { // TODO
        // Создаем абсолютно чистый новый движок SPH
        auto copy = std::make_shared<BinningRendererLayer>();

        // Поверхностно копируем weak_ptr на данные, как просили
        copy->m_dataNode = m_dataNode;

        // Сбрасываем флаги в исходное состояние, чтобы новый движок
        // честно проинициализировал свои VBO/VAO в новом контексте OpenGL
        copy->m_dirty               = true;
        copy->m_gridLineVertexCount = 0;
        copy->m_cachedGridCellSize  = -1.0;
        copy->m_particleCount       = 0;
        copy->m_hasBounds           = false;
        copy->m_boundsMin           = m_boundsMin;
        copy->m_boundsMax           = m_boundsMax;
        auto settings               = std::make_shared<BinningPointsLayerSettings>();
        copy->setSettings(settings);
        // Важно: если в конструкторе SPHRenderLayer по умолчанию создается
        // дефолтный m_settings = std::make_shared<SPHPointsLayerSettings>(),
        // то больше ничего делать не нужно — класс Layer сам заполнит его данными через VariantMap.

        return copy;
    }

    void initializeGL(QOpenGLFunctions_3_3_Core* gl) override;
    void render(QOpenGLFunctions_3_3_Core* gl, const Visualize::RenderContext& ctx) override;
    void releaseGL(QOpenGLFunctions_3_3_Core* gl) override;
    bool boundingBox(QVector3D& outMin, QVector3D& outMax) const override;

  private:
    void      buildShaders();
    void      uploadBuffersIfDirty(QOpenGLFunctions_3_3_Core* gl);
    void      ensureAccumFBO(const QSize& size);
    void      rebuildGridOverlayIfNeeded(QOpenGLFunctions_3_3_Core* gl);
    QVector3D gridCenter() const;

    QOpenGLShaderProgram m_binProgram, m_resolveProgram, m_gridOverlayProgram;
    QOpenGLBuffer m_vboPos{QOpenGLBuffer::VertexBuffer}, m_vboScalar{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject                 m_vaoParticles;
    QOpenGLBuffer                            m_vboQuad{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject                 m_vaoQuad;
    QScopedPointer<QOpenGLFramebufferObject> m_accumFBO;

    // добавлено — геометрия контура ячеек сетки (debug-оверлей через lineWidth/lineColor)
    QOpenGLBuffer            m_vboGridLines{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject m_vaoGridLines;
    int                      m_gridLineVertexCount = 0;
    double                   m_cachedGridCellSize  = -1.0;

    int       m_particleCount = 0;
    bool      m_dirty         = true;
    bool      m_visible       = true;
    bool      m_hasBounds     = false;
    QVector3D m_boundsMin, m_boundsMax;

    std::weak_ptr<Core::DataNode>               m_dataNode;
    std::shared_ptr<BinningPointsLayerSettings> m_settings;
};

} // namespace QSpace::Visualize::Layers