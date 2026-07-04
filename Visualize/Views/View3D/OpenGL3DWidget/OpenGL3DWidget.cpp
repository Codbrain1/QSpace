
#include "OpenGL3DWidget.h"
// ПРЕДПОЛОЖЕНИЕ: точный путь/имя заголовка с enum CameraViewType и ViewType —
// подставьте реальный, если отличается.
#include "Common/Enums/ViewEnums.h"
#include "Logger/Logger.h"

namespace QSpace::Visualize::Views::View3D {

OpenGL3DWidget::OpenGL3DWidget(QObject* parent)
    : AbstractView3D(parent),
      m_settings(std::make_unique<View3DSettings>()),
      m_viewport(std::make_unique<GLViewport>(m_settings.get(), nullptr)) {
}

QWidget* OpenGL3DWidget::getWidget() {
    return m_viewport.get();
}

void OpenGL3DWidget::render() {
    m_viewport->update();
}

void OpenGL3DWidget::setBackgroundColor(double r, double g, double b) {
    m_viewport->setBackgroundColor(r, g, b);
}

void OpenGL3DWidget::setAxisVisible(bool visible) {
    m_settings->axis()->setVisible(visible);
}

void OpenGL3DWidget::setGridVisible(bool visible) {
    m_settings->grid()->setVisible(visible);
}

void OpenGL3DWidget::resetCamera() {
    m_viewport->resetCamera();
}

ViewType OpenGL3DWidget::viewType() {
    // ПРЕДПОЛОЖЕНИЕ: значение enum ViewType::View3D существует в ViewEnums.h
    return ViewType::OpenGL3D;
}

void OpenGL3DWidget::setCameraView(View3D::CameraViewType cameraView) {
    // ПРЕДПОЛОЖЕНИЕ: сопоставление enum -> int индекс пресета.
    // Замените на switch по реальным значениям CameraViewType, если они
    // не совпадают с порядком в GLViewport::setCameraPreset().
    m_viewport->setCameraPreset(static_cast<int>(cameraView));
}

View3D::View3DSettings& OpenGL3DWidget::sceneSettings() {
    return *m_settings;
}

void OpenGL3DWidget::attachRenderLayer(const QUuid&                             layerId,
                                       std::shared_ptr<Visualize::IRenderLayer> layer) {
    auto glLayer = std::dynamic_pointer_cast<Visualize::IOpenGLRenderLayer>(layer);
    if (!glLayer) {
        qCWarning(LogRenderer) << "OpenGL3DWidget::attachRenderLayer: слой не реализует "
                                  "IOpenGLRenderLayer, пропущен. layerId ="
                               << layerId;
        return;
    }
    m_viewport->attachRenderLayer(layerId, glLayer);
}

void OpenGL3DWidget::detachRenderLayer(const QUuid& layerId) {
    m_viewport->detachRenderLayer(layerId);
}

} // namespace QSpace::Visualize::Views::View3D