#include "Renderer.h"
#include "Common/Logger/Logger.h"
#include <qobject.h>
#include <quuid.h>
#include <vtkCamera.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>

namespace QSpace::Visualize {
Renderer::Renderer(QObject* parent) : QObject(parent) {
    m_vtkRenderer  = vtkSmartPointer<vtkRenderer>::New();
    m_renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

    m_renderWindow->AddRenderer(m_vtkRenderer);
    m_renderWindow->SetMultiSamples(
        8); // если включить MSAA, то на AMD видеокартах будет баг с прозрачностью (частицы будут мерцать или исчезать),
            // так что отключаем его и используем только Depth Peeling для прозрачности

    auto colors = vtkSmartPointer<vtkNamedColors>::New();
    m_vtkRenderer->SetBackground(colors->GetColor3d("Black").GetData());
    // Включаем Depth Peeling для правильной прозрачности
    setupDepthPeeling();

    // Создаем оси координат
    setupAxes();
}
void Renderer::setupDepthPeeling() {
    // Без этого прозрачные частицы будут исчезать или мерцать
    m_vtkRenderer->SetUseDepthPeeling(1);
    m_vtkRenderer->SetUseFXAA(true);
    m_vtkRenderer->SetOcclusionRatio(0.1);
    m_vtkRenderer->SetMaximumNumberOfPeels(8); // 8 слоев обычно достаточно
    // Важно: в main.cpp нужно также задать AlphaBufferSize для QSurfaceFormat
}
void Renderer::setupAxes() {
    m_axesActor = vtkSmartPointer<vtkCubeAxesActor>::New();
    m_axesActor->SetCamera(m_vtkRenderer->GetActiveCamera());

    // Настройки внешнего вида осей
    m_axesActor->SetXTitle("X");
    m_axesActor->SetYTitle("Y");
    m_axesActor->SetZTitle("Z");

    // Цвет текста (Заголовки и Цифры)
    for (int i = 0; i < 3; ++i) {
        m_axesActor->GetTitleTextProperty(i)->SetColor(0, 0, 0);
        m_axesActor->GetLabelTextProperty(i)->SetColor(0, 0, 0);
    }

    // --- ЦВЕТ ЛИНИЙ ОСЕЙ (рамка куба) ---
    m_axesActor->GetXAxesLinesProperty()->SetColor(0, 0, 0);
    m_axesActor->GetYAxesLinesProperty()->SetColor(0, 0, 0);
    m_axesActor->GetZAxesLinesProperty()->SetColor(0, 0, 0);

    // --- ЦВЕТ СЕТКИ ---
    m_axesActor->GetXAxesGridlinesProperty()->SetColor(0.1, 0.1, 0.1); // Почти черный
    m_axesActor->GetYAxesGridlinesProperty()->SetColor(0.1, 0.1, 0.1);
    m_axesActor->GetZAxesGridlinesProperty()->SetColor(0.1, 0.1, 0.1);

    // Если используете внутреннюю сетку:
    m_axesActor->GetXAxesInnerGridlinesProperty()->SetColor(0.1, 0.1, 0.1);
    m_axesActor->GetYAxesInnerGridlinesProperty()->SetColor(0.1, 0.1, 0.1);
    m_axesActor->GetZAxesInnerGridlinesProperty()->SetColor(0.1, 0.1, 0.1);

    m_axesActor->SetFlyModeToStaticEdges(); // Чтобы оси не прыгали
    m_axesActor->SetVisibility(false);      // По умолчанию скрыты

    m_vtkRenderer->AddActor(m_axesActor);
}
vtkSmartPointer<vtkGenericOpenGLRenderWindow> Renderer::getRenderWindow() const {
    return m_renderWindow;
}
void Renderer::addProp(vtkSmartPointer<vtkProp> prop) {
    if (prop) {
        m_vtkRenderer->AddViewProp(prop);
    }
}
void Renderer::removeProp(vtkSmartPointer<vtkProp> prop) {
    if (prop)
        m_vtkRenderer->RemoveViewProp(prop);
}
void Renderer::addScalarBar(vtkSmartPointer<vtkScalarBarActor> bar) {
    if (bar)
        m_vtkRenderer->AddViewProp(bar);
}

void Renderer::removeScalarBar(vtkSmartPointer<vtkScalarBarActor> bar) {
    if (bar)
        m_vtkRenderer->RemoveViewProp(bar);
}
void Renderer::setBackgroundColor(double r, double g, double b) {
    m_vtkRenderer->SetBackground(r, g, b);
    // Для осей меняем цвет текста инверсно (упрощенно)
    double contrastColor = (r + g + b > 1.5) ? 0.0 : 1.0;
    if (m_axesActor) {
        for (int i = 0; i < 3; ++i) {
            m_axesActor->GetTitleTextProperty(i)->SetColor(contrastColor, contrastColor, contrastColor);
            m_axesActor->GetLabelTextProperty(i)->SetColor(contrastColor, contrastColor, contrastColor);
        }

        // Линии осей
        m_axesActor->GetXAxesLinesProperty()->SetColor(contrastColor, contrastColor, contrastColor);
        m_axesActor->GetYAxesLinesProperty()->SetColor(contrastColor, contrastColor, contrastColor);
        m_axesActor->GetZAxesLinesProperty()->SetColor(contrastColor, contrastColor, contrastColor);

        // Линии сетки (можно сделать чуть светлее/прозрачнее основного цвета для эстетики)
        double gridGray = contrastColor;
        m_axesActor->GetXAxesGridlinesProperty()->SetColor(gridGray, gridGray, gridGray);
        m_axesActor->GetYAxesGridlinesProperty()->SetColor(gridGray, gridGray, gridGray);
        m_axesActor->GetZAxesGridlinesProperty()->SetColor(gridGray, gridGray, gridGray);
    }
    render();
}
void Renderer::setAxesVisible(bool visible) {
    if (!m_axesActor)
        return;
    m_axesActor->SetVisibility(visible);
    // Обновляем границы осей по текущим данным
    if (visible) {
        m_vtkRenderer->ResetCameraClippingRange();
        double bounds[6];
        m_vtkRenderer->ComputeVisiblePropBounds(bounds);
        m_axesActor->SetBounds(bounds);
    }
    render();
}
void Renderer::setGridVisible(bool visible) {
    if (!m_axesActor)
        return;
    m_axesActor->SetDrawXGridlines(visible);
    m_axesActor->SetDrawYGridlines(visible);
    m_axesActor->SetDrawZGridlines(visible);
    // TODO: добавить настройку внешнего вида сетки
    render();
}
void Renderer::setCameraView(CameraViewType view) {
    vtkCamera* cam  = m_vtkRenderer->GetActiveCamera();
    double     dist = cam->GetDistance();
    double     fp[3];
    cam->GetFocalPoint(fp);

    switch (view) {
        case CameraViewType::XY_Top:
            cam->SetPosition(fp[0], fp[1], fp[2] + dist);
            cam->SetViewUp(0, 1, 0);
            break;
        case CameraViewType::XZ_Front:
            cam->SetPosition(fp[0], fp[1] - dist, fp[2]);
            cam->SetViewUp(0, 0, 1);
            break;
        case CameraViewType::YZ_Right:
            cam->SetPosition(fp[0] + dist, fp[1], fp[2]);
            cam->SetViewUp(0, 0, 1);
            break;
        case CameraViewType::Iso:
            cam->SetPosition(fp[0] + dist, fp[1] - dist, fp[2] + dist);
            cam->SetViewUp(0, 0, 1);
            break;
    }
    m_vtkRenderer->ResetCameraClippingRange();
    render();
}
void Renderer::setCenterOfRotation(double x, double y, double z) {
    vtkCamera* cam = m_vtkRenderer->GetActiveCamera();

    // Перемещаем фокальную точку камеры (куда она смотрит)
    // Это создает эффект переноса системы координат в центр масс
    cam->SetFocalPoint(x, y, z);

    // Сдвигаем камеру так, чтобы сохранить дистанцию
    // (тут простая логика, можно усложнить)
    render();
}
void Renderer::resetCamera() {
    if (m_vtkRenderer) {
        qCInfo(LogRenderer) << "Renderer::resetCamera() - Resetting camera";
        m_vtkRenderer->ResetCamera();
        double bounds[6] = {0};
        m_vtkRenderer->ComputeVisiblePropBounds(bounds);
        qCInfo(LogRenderer) << "Renderer::resetCamera() - Bounds:" << bounds[0] << "-" << bounds[1] << "," << bounds[2]
                            << "-" << bounds[3] << "," << bounds[4] << "-" << bounds[5];
    }
}
void Renderer::render() {
    qCDebug(LogRenderer) << "Renderer::render() called";
    if (m_renderWindow) {
        m_renderWindow->Modified();
    }
    emit updateRequested();
}
void Renderer::renderForce() {
    // TODO:: может привести к непонятным последствиям при рендеринге или замедлению его работы
    if (m_renderWindow && !m_renderWindow->GetNeverRendered()) {
        m_renderWindow->Render();
    }
}
} // namespace QSpace::Visualize