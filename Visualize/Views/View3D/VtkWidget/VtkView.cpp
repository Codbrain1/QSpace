#include "VtkView.h"
#include "Common/Logger/Logger.h"
#include <qelapsedtimer.h>
#include <qloggingcategory.h>
#include <qobject.h>
#include <quuid.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCameraPass.h>
#include <vtkCaptionActor2D.h>
#include <vtkCubeAxesActor.h>
#include <vtkFramebufferPass.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkPointPicker.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkRenderStepsPass.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSmartPointerBase.h>
#include <vtkTextProperty.h>
#include <vtkToneMappingPass.h>
#include "GalacticInteractorStyle.h"
#include <cstddef>

namespace QSpace::Visualize {
VtkView::VtkView(QObject* parent) : IView3D(parent) {
    m_vtkWidget   = std::make_unique<QVTKOpenGLNativeWidget>();
    m_vtkRenderer = vtkSmartPointer<vtkRenderer>::New();

    m_renderWindow = vtkGenericOpenGLRenderWindow::SafeDownCast(m_vtkWidget->renderWindow());
    m_renderWindow->AddRenderer(m_vtkRenderer);

    auto colors = vtkSmartPointer<vtkNamedColors>::New();
    m_vtkRenderer->SetBackground(colors->GetColor3d("Black").GetData());
    // 2. Получаем и настраиваем интерактор напрямую из окна виджета
    if (auto interactor = m_renderWindow->GetInteractor()) {
        // Настраиваем точечный пикер
        vtkNew<vtkPointPicker> pointPicker;
        pointPicker->SetTolerance(0.005);
        interactor->SetPicker(pointPicker);

        // Устанавливаем кастомный стиль (галактический)
        vtkNew<GalacticInteractorStyle> style;
        interactor->SetInteractorStyle(style);

        // Важно: Инициализируем интерактор до добавления виджетов
        interactor->Initialize();

        // 3. Настройка осей и Orientation Marker
        vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
        axes->SetTotalLength(1.5, 1.5, 1.5);
        axes->SetShaftTypeToCylinder();
        axes->SetCylinderRadius(0.02);
        axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1.0, 0.0, 0.0);
        axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(0.0, 1.0, 0.0);
        axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(0.0, 0.0, 1.0);

        axes->GetXAxisCaptionActor2D()->LeaderOff();
        axes->GetXAxisCaptionActor2D()->BorderOff();
        axes->GetYAxisCaptionActor2D()->LeaderOff();
        axes->GetYAxisCaptionActor2D()->BorderOff();
        axes->GetZAxisCaptionActor2D()->LeaderOff();
        axes->GetZAxisCaptionActor2D()->BorderOff();

        m_orientationMarker = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
        m_orientationMarker->SetOrientationMarker(axes);
        m_orientationMarker->SetInteractor(interactor); // Теперь interactor валиден!
        m_orientationMarker->SetViewport(0.0, 0.0, 0.3, 0.3);
        m_orientationMarker->SetEnabled(1);
    }

    // 4. Настройки рендеринга и прозрачности
    m_renderWindow->SetAlphaBitPlanes(1);
    setupDepthPeeling();

    // 5. Создаем сетку/оси сцены
    setupAxes();
    if (m_axesActor) {
        m_axesActor->SetCamera(m_vtkRenderer->GetActiveCamera());
    }
}

QWidget* VtkView::getWidget() {
    return m_vtkWidget.get();
}

void VtkView::setRenderWindow(vtkRenderWindow* renderWindow) {
    if (!renderWindow)
        return;
    if (m_renderWindow) {
        m_renderWindow->RemoveRenderer(m_vtkRenderer);
    }
    m_renderWindow = vtkGenericOpenGLRenderWindow::SafeDownCast(renderWindow);
    if (m_renderWindow) {
        m_renderWindow->AddRenderer(m_vtkRenderer);
        if (auto interactor = m_renderWindow->GetInteractor()) {
            // 1. Создаем и настраиваем точечный пикер
            vtkNew<vtkPointPicker> pointPicker;
            // Допуск (tolerance) — насколько точно нужно попасть в точку.
            // Для малых частиц можно чуть увеличить.
            pointPicker->SetTolerance(0.005);
            interactor->SetPicker(pointPicker);

            // 2. Устанавливаем стиль
            vtkNew<GalacticInteractorStyle> style;
            interactor->SetInteractorStyle(style);

            interactor->Initialize();

            vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
            axes->SetTotalLength(1.5, 1.5, 1.5);
            axes->SetShaftTypeToCylinder(); // Делаем стрелки объемными цилиндрами
            axes->SetCylinderRadius(0.02);
            axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1.0,
                                                                               0.0,
                                                                               0.0); // Красный X
            axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(0.0,
                                                                               1.0,
                                                                               0.0); // Зеленый Y
            axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(0.0,
                                                                               0.0,
                                                                               1.0); // Синий Z
            m_orientationMarker = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
            m_orientationMarker->SetOrientationMarker(axes);
            m_orientationMarker->SetInteractor(interactor);
            m_orientationMarker->SetViewport(0.0, 0.0, 0.3, 0.3);
            m_orientationMarker->SetEnabled(1);
            axes->GetXAxisCaptionActor2D()->LeaderOff();
            axes->GetXAxisCaptionActor2D()->BorderOff();
            axes->GetYAxisCaptionActor2D()->LeaderOff();
            axes->GetYAxisCaptionActor2D()->BorderOff();
            axes->GetZAxisCaptionActor2D()->LeaderOff();
            axes->GetZAxisCaptionActor2D()->BorderOff();
        }
        m_renderWindow->SetAlphaBitPlanes(1);
        // Включаем Depth Peeling для правильной прозрачности
        setupDepthPeeling();

        if (m_axesActor) {
            m_axesActor->SetCamera(m_vtkRenderer->GetActiveCamera());
        }
        render();
    }
}

void VtkView::setupDepthPeeling() {
    if (!m_vtkRenderer || !m_renderWindow)
        return;

    m_renderWindow->SetMultiSamples(0);
    m_renderWindow->SetUseSRGBColorSpace(true);
    m_vtkRenderer->SetUseDepthPeeling(1);
    m_vtkRenderer->SetUseSSAO(false);
    m_vtkRenderer->SetMaximumNumberOfPeels(4); // Максимальное количество проходов для peeling
    m_vtkRenderer->SetUseFXAA(true);

    vtkNew<vtkRenderStepsPass> steps;
    vtkNew<vtkFramebufferPass> fbPass;
    fbPass->SetDelegatePass(steps);
    vtkNew<vtkToneMappingPass> toneMapping;
    toneMapping->SetToneMappingType(vtkToneMappingPass::GenericFilmic);
    toneMapping->SetDelegatePass(fbPass);
    toneMapping->SetExposure(1); // Можно вынести в UI как "Яркость сцены"
    // toneMapping->SetContrast(1); // Можно вынести в UI как "Контраст сцены"
    m_vtkRenderer->SetPass(toneMapping);
}

void VtkView::setGlobalExposure(double exposure) {
    vtkRenderPass* pass = m_vtkRenderer->GetPass();
    if (auto toneMappingPass = vtkToneMappingPass::SafeDownCast(pass)) {
        toneMappingPass->SetExposure(exposure);
        toneMappingPass->Modified();
        // m_vtkRenderer->SetPass(toneMappingPass);
        render();
    }
}

void VtkView::setupAxes() {
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

vtkSmartPointer<vtkGenericOpenGLRenderWindow> VtkView::getRenderWindow() const {
    return m_renderWindow;
}

void VtkView::addProp(vtkSmartPointer<vtkProp> prop) {
    if (prop) {
        m_vtkRenderer->AddViewProp(prop);
    }
}

void VtkView::removeProp(vtkSmartPointer<vtkProp> prop) {
    if (prop)
        m_vtkRenderer->RemoveViewProp(prop);
}

void VtkView::setBackgroundColor(double r, double g, double b) {
    m_vtkRenderer->SetBackground(r, g, b);
    // Для осей меняем цвет текста инверсно (упрощенно)
    double contrastColor = (r + g + b > 1.5) ? 0.0 : 1.0;
    if (m_axesActor) {
        for (int i = 0; i < 3; ++i) {
            m_axesActor->GetTitleTextProperty(i)->SetColor(contrastColor,
                                                           contrastColor,
                                                           contrastColor);
            m_axesActor->GetLabelTextProperty(i)->SetColor(contrastColor,
                                                           contrastColor,
                                                           contrastColor);
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
    double contrast = (r + g + b > 1.5) ? 0.0 : 1.0;
    emit   backgroundColorChanged(contrast);
    render();
}

void VtkView::setAxesVisible(bool visible) {
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

void VtkView::setGridVisible(bool visible) {
    if (!m_axesActor)
        return;
    m_axesActor->SetDrawXGridlines(visible);
    m_axesActor->SetDrawYGridlines(visible);
    m_axesActor->SetDrawZGridlines(visible);
    // TODO: добавить настройку внешнего вида сетки
    render();
}

void VtkView::setCameraView(CameraViewType view) {
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

void VtkView::setCenterOfRotation(double x, double y, double z) {
    vtkCamera* cam = m_vtkRenderer->GetActiveCamera();

    // Перемещаем фокальную точку камеры (куда она смотрит)
    // Это создает эффект переноса системы координат в центр масс
    cam->SetFocalPoint(x, y, z);

    // Сдвигаем камеру так, чтобы сохранить дистанцию
    // (тут простая логика, можно усложнить)
    render();
}

void VtkView::resetCamera() {
    if (m_vtkRenderer) {
        qCInfo(LogRenderer) << "VtkView::resetCamera() - Resetting camera";
        m_vtkRenderer->ResetCamera();
        double bounds[6] = {0};
        m_vtkRenderer->ComputeVisiblePropBounds(bounds);
        qCInfo(LogRenderer) << "VtkView::resetCamera() - Bounds:" << bounds[0] << "-" << bounds[1]
                            << "," << bounds[2] << "-" << bounds[3] << "," << bounds[4] << "-"
                            << bounds[5];
    }
    render();
}

void VtkView::render() {
#ifdef QSPACE_PROJECT_DEBUG_MODE
    QElapsedTimer timer;
    timer.start();
#endif
    if (m_vtkWidget && m_renderWindow) {
        m_renderWindow->Modified(); // Маркируем окно как измененное конвейером
        m_vtkWidget->update();      // Делегируем обновление Qt
    }
#ifdef QSPACE_PROJECT_DEBUG_MODE
    qint64 time = timer.elapsed();
    qCInfo(LogRenderer) << "Time Rendering: " << time << " milliseconds";
#endif
}

void VtkView::renderForce() {
#ifdef QSPACE_PROJECT_DEBUG_MODE
    QElapsedTimer timer;
    timer.start();
#endif
    if (m_renderWindow && !m_renderWindow->GetNeverRendered()) {
        m_renderWindow->Render();
    }
#ifdef QSPACE_PROJECT_DEBUG_MODE
    qint64 time = timer.elapsed();
    qCInfo(LogRenderer) << "Time Rendering Force: " << time << " milliseconds";
#endif
}

vtkRenderWindowInteractor* VtkView::getInteractor() const {
    if (m_renderWindow) {
        return m_renderWindow->GetInteractor();
    }
    return nullptr;
}
} // namespace QSpace::Visualize