// Visualize/Layers/SPHRendererLayer.h
#pragma once
#include "Common/Interfaces/IRenderLayer.h"
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QScopedPointer>
#include "SPHPointsLayerSettings.h"

namespace QSpace::Visualize::Layers {

class SPHRendererLayer : public IOpenGLRenderLayer {
  public:
    void update() override {
        m_dirty = true;
    }

    void setData(std::weak_ptr<Core::DataNode> node) override {
        m_dataNode = node;
        m_dirty    = true;
    }

    void setSettings(std::shared_ptr<LayerSettings> settings) override {
        m_settings = std::dynamic_pointer_cast<SPHPointsLayerSettings>(settings);
    }

    void setVisible(bool visible) override {
        m_visible = visible;
    }

    bool isVisible() const override {
        return m_visible;
    }

    void initializeGL(QOpenGLFunctions_3_3_Core* gl) override;
    void render(QOpenGLFunctions_3_3_Core* gl, const Visualize::RenderContext& ctx) override;
    void releaseGL(QOpenGLFunctions_3_3_Core* gl) override;
    bool boundingBox(QVector3D& outMin, QVector3D& outMax) const override;

  private:
    void buildShaders();
    void uploadBuffersIfDirty(QOpenGLFunctions_3_3_Core* gl);
    void ensureAccumFBO(const QSize& size);
    void calibrateIfNeeded(QOpenGLFunctions_3_3_Core* gl);

    QOpenGLShaderProgram m_splatProgram, m_resolveProgram;
    QOpenGLBuffer m_vboPos{QOpenGLBuffer::VertexBuffer}, m_vboMass{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject                 m_vaoParticles;
    QOpenGLBuffer                            m_vboQuad{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject                 m_vaoQuad;
    QScopedPointer<QOpenGLFramebufferObject> m_accumFBO;

    int  m_particleCount    = 0;
    bool m_dirty            = true;
    bool m_visible          = true;
    bool m_needsCalibration = true;

    std::weak_ptr<Core::DataNode>           m_dataNode;
    std::shared_ptr<SPHPointsLayerSettings> m_settings;
};

} // namespace QSpace::Visualize::Layers