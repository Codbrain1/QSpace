#include "BaseLayer.h"
#include "Common/Logger/Logger.h"
#include "Common/Structures/RenderStructures.h"
#include "QSpaceScalarBar.h"
#include "Visualize/ColorMapManager/ColorMapManager.h"
#include <memory>
#include <vtkAbstractMapper.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPolyData.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkScalarBarRepresentation.h>
#include <vtkStringArray.h>
#include <vtkTextProperty.h>

namespace QSpace::Visualize {
BaseLayer::BaseLayer(std::shared_ptr<QSpace::Core::DataNode> node) : m_node(node) {
    m_lut             = vtkSmartPointer<vtkColorTransferFunction>::New();
    m_opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
    m_scalarBar       = vtkSmartPointer<QSpaceScalarBar>::New();
    m_scalarBarWidget = nullptr; // Инициализируем как nullptr, будет создан при attachInteractor
}
void BaseLayer::attachInteractor(vtkRenderWindowInteractor* interactor) {
    if (!interactor)
        return;
    if (!m_scalarBarWidget) {
        m_scalarBarWidget = vtkSmartPointer<vtkScalarBarWidget>::New();
        m_scalarBarWidget->SetScalarBarActor(m_scalarBar);
        auto rep = vtkScalarBarRepresentation::SafeDownCast(m_scalarBarWidget->GetRepresentation());
        if (rep) {
            rep->SetShowBorderToActive();
        }
    }
    m_scalarBarWidget->EnabledOff(); // Сначала отключаем, включим позже при необходимости
    m_scalarBarWidget->SetInteractor(interactor);
    if (m_node->settings.showScalarBar && m_node->settings.isVisible) {
        m_scalarBarWidget->EnabledOn();
    }
}
void BaseLayer::detachInteractor() {
    if (m_scalarBarWidget) {
        m_scalarBarWidget->EnabledOff();
        m_scalarBarWidget->SetInteractor(nullptr);
    }
}
void BaseLayer::updateColorsForContrast(double contrast) {
    if (!m_scalarBar)
        return;

    double color[3] = {contrast, contrast, contrast};

    m_scalarBar->GetTitleTextProperty()->SetColor(color);
    m_scalarBar->GetLabelTextProperty()->SetColor(color);
    m_scalarBar->GetAnnotationTextProperty()->SetColor(color);

    // Если есть рамка или другие элементы в репрезентации виджета:
    if (m_scalarBarWidget) {
        auto rep = vtkScalarBarRepresentation::SafeDownCast(m_scalarBarWidget->GetRepresentation());
        if (rep) {
            rep->SetShowBorderToActive();
            if (auto borderProp = rep->GetBorderProperty()) {
                borderProp->SetColor(color);
                borderProp->SetLineWidth(2.0);
            }
        }
    }
}
void BaseLayer::update() {
    if (!m_node || !m_node->data) {
        qCCritical(LogRenderer) << "ParticleLayer::update() - Node or data is null";
        return;
    }

    auto data = vtkPolyData::SafeDownCast(m_node->data);
    if (!data || data->GetNumberOfPoints() == 0)
        return;

    auto& s = m_node->settings;
    m_baseProp->SetVisibility(s.isVisible);

    if (m_baseMapper->GetInputDataObject(0, 0) != data) {
        m_baseMapper->SetInputDataObject(data);
    }

    applyRenderModeSettings(s);
    setupDataArrays(s);
    updateScalarBarVisibility(s);
    postUpdate(s);
}
void BaseLayer::applyColorMap(const QUuid& colorMapUuid, double range[2]) {
    m_lut->RemoveAllPoints();
    m_opacityFunction->RemoveAllPoints();

    double minVal = range[0];
    double maxVal = range[1];
    double span   = maxVal - minVal + 1e-9;

    auto& cmManager = ColorMapManager::instance();
    auto  colorMap  = cmManager.getMap(colorMapUuid).value_or(ColorMapPresets::getStandardPresets().first());

    // Временная LUT для интерполяции базовых цветов
    auto tempLut = vtkSmartPointer<vtkColorTransferFunction>::New();
    tempLut->SetColorSpaceToLab();
    for (const auto& pt : colorMap.points)
        tempLut->AddRGBPoint(pt.x, pt.r, pt.g, pt.b);

    const int numSamples    = 256;
    auto      interpColor   = scalarBarRangeInterpolationFromString(m_node->settings.interpolationRangeType);
    auto      interpOpacity = interpolationOpacityFunctionFromString(m_node->settings.interpolationOpacityFunction);
    for (int i = 0; i < numSamples; ++i) {
        double t   = static_cast<double>(i) / (numSamples - 1);
        double val = minVal + t * (maxVal - minVal);

        // --- 1. Интерполяция Цвета (сохранение всех типов) ---
        double t_color = t;
        if (interpColor == ScalarBarRangeInterpolation::Sigmoid) {
            double g   = m_node->settings.sigmoidGammaColor;
            double s   = m_node->settings.sigmoidShiftColor;
            auto   sig = [&](double x) { return 1.0 / (1.0 + std::exp(-g * (x - s))); };
            t_color    = (sig(t) - sig(0.0)) / (sig(1.0) - sig(0.0) + 1e-9);
        } else if (interpColor == ScalarBarRangeInterpolation::Asinh) {
            double a = m_node->settings.alpha;
            t_color  = std::asinh(t * a) / std::asinh(a);
        }
        t_color = std::clamp(t_color, 0.0, 1.0);

        double rgb[3];
        tempLut->GetColor(t_color, rgb);

        // --- 2. Интерполяция Прозрачности (сохранение всех 6+ типов) ---
        double baseAlpha = m_node->settings.opacity;
        double t_opacity = 1.0;

        if (interpOpacity == InterpolationOpacityFunction::Sigmoid) {
            double g   = m_node->settings.sigmoidGammaOpacity;
            double s   = m_node->settings.sigmoidShiftOpacity;
            auto   sig = [&](double x) { return 1.0 / (1.0 + std::exp(-g * (x - s))); };
            t_opacity  = (sig(t) - sig(0.0)) / (sig(1.0) - sig(0.0) + 1e-9);
        } else if (interpOpacity == InterpolationOpacityFunction::Asinh) {
            t_opacity = std::asinh(t * baseAlpha) / std::asinh(baseAlpha);
        } else if (interpOpacity == InterpolationOpacityFunction::Linear) {
            t_opacity = t;
        } else if (interpOpacity == InterpolationOpacityFunction::Quadro) {
            t_opacity = t * t;
        } else if (interpOpacity == InterpolationOpacityFunction::Qube) {
            t_opacity = t * t * t;
        } else if (interpOpacity == InterpolationOpacityFunction::Sqrt) {
            t_opacity = std::sqrt(t);
        } else if (interpOpacity == InterpolationOpacityFunction::Constant) {
            t_opacity = 1.0;
        }

        double finalOpacity = std::clamp(baseAlpha * t_opacity, 0.0, 1.0);

        m_lut->AddRGBPoint(val, rgb[0], rgb[1], rgb[2]);
        m_opacityFunction->AddPoint(val, finalOpacity);
    }

    // Обработка выхода за диапазон
    if (m_node->settings.hideOutOfRange) {
        double eps = span * 0.001;
        m_opacityFunction->AddPoint(minVal - eps, 0.0);
        m_opacityFunction->AddPoint(maxVal + eps, 0.0);
    }
}
void BaseLayer::setupLayer() {
    m_lut->SetVectorModeToMagnitude();  // для корректной работы с векторными полями (если будут)
    m_lut->SetColorSpaceToLab();        // более плавные градиенты для восприятия
    setupScalarBar();                   // базовая настройка легенды, будет донастраиваться при обновлении
    m_scalarBar->SetLookupTable(m_lut); // связываем легенду с нашей цветовой картой
    m_scalarBar->SetUseOpacity(true);   // включаем использование функции прозрачности в легенде
    if (m_node && m_node->data &&
        m_node->data->GetPointData()->GetNumberOfArrays() > 0) { // если есть массивы для отрисовки
        m_node->settings.colorByField = m_node->data->GetPointData()->GetArrayName(0);
    }
}
void BaseLayer::setupScalarBar() {
    // TODO: параметризовать настройку colorBar
    m_scalarBar->SetNumberOfLabels(6);
    m_scalarBar->SetWidth(0.1);  // 10% ширины экрана
    m_scalarBar->SetHeight(0.4); // 50% высоты экрана
    m_scalarBar->SetVerticalTitleSeparation(15);
    // Позиция справа
    m_scalarBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    m_scalarBar->GetPositionCoordinate()->SetValue(0.85, 0.05);
    // Настройка текста
    vtkTextProperty* txt = m_scalarBar->GetLabelTextProperty();
    txt->SetColor(1, 1, 1); // Черный текст
    txt->SetFontSize(16);
    m_scalarBar->GetTitleTextProperty()->SetColor(1, 1, 1);
}
void BaseLayer::updateScalarBarVisibility(const Core::VisualSettings& s) {
    bool shouldShow = s.showScalarBar && s.isVisible;
    m_scalarBar->SetVisibility(shouldShow);
    m_scalarBarWidget->SetEnabled(shouldShow);
    if (!shouldShow)
        return;
    m_scalarBar->SetLookupTable(m_lut);
    m_scalarBar->SetTitle(s.colorByField.toStdString().c_str());
    // Если режим выключен, возвращаем стандартные метки
    m_scalarBar->SetLogMode(s.useLogScale);
    m_scalarBar->GetAnnotationTextProperty()->ShallowCopy(m_scalarBar->GetLabelTextProperty());
}
void BaseLayer::swapData(std::shared_ptr<QSpace::Core::DataNode> node) {
    if (!node || !node->data)
        return;
    auto polyData = vtkPolyData::SafeDownCast(node->data);
    if (!polyData)
        return;
    auto oldSettings = m_node->settings;
    m_node           = node;
    m_node->settings = oldSettings;
    m_baseMapper->SetInputDataObject(0, polyData);
    m_baseMapper->Modified();
    update();
}
void BaseLayer::setVisible(bool visible) {
    // 1. Управляем видимостью основного актора
    if (m_baseProp) {
        m_baseProp->SetVisibility(visible ? 1 : 0);
    }
    // 2. Управляем виджетом легенды (если он есть)
    if (m_scalarBarWidget) {
        if (visible && m_node->settings.showScalarBar) {
            m_scalarBarWidget->EnabledOn();
        } else {
            m_scalarBarWidget->EnabledOff();
        }
    }
    // Обновляем состояние в структуре настроек, чтобы оно синхронизировалось
    m_node->settings.isVisible = visible;
}
bool BaseLayer::isVisible() const {
    if (m_baseProp) {
        return m_baseProp->GetVisibility() != 0;
    }
    return false;
}
} // namespace QSpace::Visualize