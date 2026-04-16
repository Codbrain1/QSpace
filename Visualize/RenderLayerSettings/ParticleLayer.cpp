#include "ParticleLayer.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Logger/Logger.h"
#include "Common/Structures/CoreStructures.h"
#include "Common/Structures/RenderStructures.h"
#include "Visualize/ColorMapManager/ColorMapManager.h"
#include <cmath>
#include <memory>
#include <qloggingcategory.h>
#include <qstring.h>
#include <quuid.h>
#include <vtkAbstractArray.h>
#include <vtkActor.h>
#include <vtkCell.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkMappedDataArray.h>
#include <vtkPointData.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkScalarBarRepresentation.h>
#include <vtkScalarBarWidget.h>
#include <vtkShaderProperty.h>
#include <vtkSmartPointer.h>
#include <vtkSmartPointerBase.h>
#include <vtkTextProperty.h>
#include <vtkType.h>

namespace QSpace::Visualize {
ParticleLayer::ParticleLayer(std::shared_ptr<Core::DataNode> node) : m_node(node) {
    m_mapper          = vtkSmartPointer<vtkPointGaussianMapper>::New();
    m_actor           = vtkSmartPointer<vtkActor>::New();
    m_lut             = vtkSmartPointer<vtkColorTransferFunction>::New();
    m_scalarBar       = vtkSmartPointer<vtkScalarBarActor>::New();
    m_opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

    // Инициализация пайплайна
    m_mapper->SetStatic(true);
    m_mapper->SetEmissive(false);
    m_actor->SetMapper(m_mapper);

    // Настройка маппинга
    m_mapper->ScalarVisibilityOn();
    m_mapper->SetScalarModeToUsePointFieldData();
    m_mapper->SetColorModeToMapScalars();

    m_lut->SetVectorModeToMagnitude();
    m_lut->SetColorSpaceToLab();

    setupScalarBar();
    m_scalarBar->SetLookupTable(m_lut);
    m_scalarBar->SetUseOpacity(true);

    m_actor->PickableOn();

    if (m_node && m_node->data && m_node->data->GetPointData()->GetNumberOfArrays() > 0) {
        m_node->settings.colorByField = m_node->data->GetPointData()->GetArrayName(0);
    }
}
void ParticleLayer::updateColorsForContrast(double contrast) {
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
        }
    }
}
void ParticleLayer::update() {
    if (!m_node || !m_node->data) {
        qCCritical(LogRenderer) << "ParticleLayer::update() - Node or data is null";
        return;
    }

    auto data = vtkPolyData::SafeDownCast(m_node->data);
    if (!data || data->GetNumberOfPoints() == 0)
        return;

    auto& s = m_node->settings;
    m_actor->SetVisibility(s.isVisible);

    if (m_mapper->GetInput() != data) {
        m_mapper->SetInputData(data);
    }

    applyRenderModeSettings(s);
    setupDataArrays(data, s);
    updateScalarBarVisibility(s);

    if (s.mode == RenderMode::GausianSplat) {
        updateShader(s);
    }
}
void ParticleLayer::applyRenderModeSettings(const Core::VisualSettings& s) {
    if (s.mode == RenderMode::GausianSplat) {
        m_mapper->SetScaleFactor(s.PointSize);
        m_mapper->SetScalarOpacityFunction(m_opacityFunction);
        m_mapper->SetEmissive(s.isEmmisive);
    } else if (s.mode == RenderMode::Points) {
        m_mapper->SetScaleFactor(0.0);
        m_actor->GetProperty()->SetPointSize(s.PointSize);
    }
    bool isDensityLike = s.colorByField.contains("mass", Qt::CaseInsensitive) ||
                         s.colorByField.contains("rho", Qt::CaseInsensitive) ||
                         s.colorByField.contains("density", Qt::CaseInsensitive) ||
                         s.colorByField.contains("energy", Qt::CaseInsensitive);

    isDensityLike ? m_actor->ForceTranslucentOn() : m_actor->ForceTranslucentOff();
    m_actor->GetProperty()->SetOpacity(s.opacity);
}
void ParticleLayer::setupDataArrays(vtkPolyData* data, Core::VisualSettings& s) {
    if (s.colorByField.isEmpty()) {
        m_mapper->ScalarVisibilityOff();
        m_actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
        return;
    }
    std::string sourceField         = s.colorByField.toStdString();
    std::string currentWorkingField = sourceField;
    if (s.colorByField.contains("energy", Qt::CaseInsensitive)) {
        std::string dimField = "Dimension_" + sourceField;
        if (!data->GetPointData()->HasArray(dimField.c_str())) {
            auto calc = vtkSmartPointer<vtkArrayCalculator>::New();
            calc->SetInputData(data);
            calc->AddScalarVariable("v", sourceField.c_str(), 0);
            // Сохранена оригинальная формула SPH интерполяции
            calc->SetFunction("sqrt(1.66666666667 * 0.66666666667*v)^2 * 1.79E+6");
            calc->SetResultArrayName(dimField.c_str());
            calc->Update();
            data->GetPointData()->AddArray(calc->GetDataSetOutput()->GetPointData()->GetArray(dimField.c_str()));
        }
        currentWorkingField = dimField;
    }
    vtkDataArray* arr_temp = data->GetPointData()->GetArray(currentWorkingField.c_str());
    int           comp     = (arr_temp->GetNumberOfComponents() == 3) ? -1 : 0;
    double        range[2] = {0.0, 1.0};
    arr_temp->GetRange(range, comp);

    // устанавливаем базовый диапазон для возможности его использования при авто-диапазоне и в UI
    s.baseRangeMin         = range[0];
    s.baseRangeMax         = range[1];
    std::string finalField = currentWorkingField;
    if (s.useLogScale) {
        finalField = "Log_" + currentWorkingField;
        if (!data->GetPointData()->HasArray(finalField.c_str())) {
            auto calc = vtkSmartPointer<vtkArrayCalculator>::New();
            calc->SetInputData(data);
            calc->AddScalarVariable("v", currentWorkingField.c_str(), 0);
            calc->SetFunction("log10(abs(v) + 1e-10)");
            calc->SetResultArrayName(finalField.c_str());
            calc->Update();
            data->GetPointData()->AddArray(calc->GetDataSetOutput()->GetPointData()->GetArray(finalField.c_str()));
        }
    }
    vtkDataArray* arr = data->GetPointData()->GetArray(finalField.c_str());
    if (!arr)
        return;

    data->GetPointData()->SetActiveScalars(finalField.c_str());
    m_mapper->SelectColorArray(finalField.c_str());
    m_mapper->SetOpacityArray(finalField.c_str());
    m_mapper->SetArrayComponent(comp);
    m_mapper->SetOpacityArrayComponent(comp);
    m_mapper->SetLookupTable(m_lut);

    if (s.autoRange) {
        arr->GetRange(range, comp);
        s.rangeMin = s.baseRangeMin;
        s.rangeMax = s.baseRangeMax;
    } else {
        range[0] = s.rangeMin;
        range[1] = s.rangeMax;
        if (s.useLogScale) {
            range[0] = std::log10(std::abs(s.rangeMin) + 1e-10);
            range[1] = std::log10(std::abs(s.rangeMax) + 1e-10);
        }
    }
    applyColorMap(s.colorMapId, range);
    m_mapper->SetScalarRange(range);
}
void ParticleLayer::applyColorMap(QUuid& colorMapUuid, double range[2]) {
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
void ParticleLayer::updateShader(const Core::VisualSettings& s) {
    if (shaderTypeFromString(s.ShaderType) == ShaderType::Default) {
        m_mapper->SetSplatShaderCode(nullptr);
        return;
    }

    // Улучшенный шейдер для SPH: добавлена имитация сферичности (fake normals)
    // и корректное накопление плотности.
    QString shaderCode = QString("//VTK::Color::Impl\n"
                                 "float dist2 = dot(offsetVCVSOutput.xy, offsetVCVSOutput.xy);\n"
                                 "if (dist2 > 1.0) discard;\n"

                                 // SPH Улучшение: вычисление "нормали" спрайта для псевдо-освещения
                                 "float z = sqrt(1.0 - dist2);\n"
                                 "float diffuse = max(0.0, z); \n" // Свет сверху

                                 "float gaussian = exp(-%1 * dist2);\n"
                                 "opacity = opacity * gaussian;\n"
                                 "vec3 baseColor = vertexColorVSOutput.rgb;\n"

                                 "if (%2) {\n" // Emissive
                                 "    ambientColor = baseColor * opacity * %3;\n"
                                 "    diffuseColor = vec3(0.0);\n"
                                 "} else {\n" // SPH Shaded
                                 "    diffuseColor = baseColor * diffuse;\n"
                                 "    ambientColor = baseColor * opacity * 0.5;\n"
                                 "}\n")
                             .arg(s.gaussianSharpness)
                             .arg(s.isEmmisive ? "true" : "false")
                             .arg(s.exposureClamp);

    m_mapper->SetSplatShaderCode(shaderCode.toUtf8().constData());
}
void ParticleLayer::updateScalarBarVisibility(const Core::VisualSettings& s) {
    bool shouldShow = s.showScalarBar && s.isVisible;
    m_scalarBar->SetVisibility(shouldShow);
    m_scalarBarWidget->SetEnabled(shouldShow);
    if (!shouldShow)
        return;
    m_scalarBar->SetLookupTable(m_lut);
    m_scalarBar->SetTitle(s.colorByField.toStdString().c_str());
    // Если режим выключен, возвращаем стандартные метки
    if (!s.useLogScale) {
        m_scalarBar->DrawTickLabelsOn();
        m_scalarBar->DrawAnnotationsOff();
        m_scalarBar->SetTextPositionToSucceedScalarBar();
    } else {
        m_scalarBar->DrawTickLabelsOff();
        m_scalarBar->DrawAnnotationsOn();
        vtkScalarsToColors* lut = m_scalarBar->GetLookupTable();
        if (!lut)
            return;
        // double range[2];
        auto range = lut->GetRange();
        // Очищаем старые аннотации
        vtkSmartPointer<vtkStringArray> annNames  = vtkSmartPointer<vtkStringArray>::New();
        vtkSmartPointer<vtkDoubleArray> annValues = vtkSmartPointer<vtkDoubleArray>::New();
        int                             numTicks  = m_scalarBar->GetNumberOfLabels();
        for (int i = 0; i < numTicks; ++i) {
            // Линейно распределяем значения в лог-пространстве (например 0, 1, 2, 3...)
            double t      = (double)i / (numTicks - 1);
            double logVal = range[0] + t * (range[1] - range[0]);
            // Проводим операцию 10^x
            double physVal = std::pow(10.0, logVal);
            // Форматируем текст (научная нотация)
            std::stringstream ss;
            ss << std::scientific << std::setprecision(1) << physVal;
            annValues->InsertNextValue(logVal);
            annNames->InsertNextValue(ss.str());
        }
        m_scalarBar->SetTextPositionToPrecedeScalarBar();
        lut->SetAnnotations(annValues, annNames);
    }
    m_scalarBar->GetAnnotationTextProperty()->ShallowCopy(m_scalarBar->GetLabelTextProperty());
}
void ParticleLayer::swapData(std::shared_ptr<QSpace::Core::DataNode> node) {
    if (!node || !node->data)
        return;
    auto polyData = vtkPolyData::SafeDownCast(node->data);
    if (!polyData)
        return;
    auto oldSettings = m_node->settings;
    m_node           = node;
    m_node->settings = oldSettings;
    m_mapper->SetInputData(polyData);
    m_mapper->Modified();
    update();
}
void ParticleLayer::setupScalarBar() {
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
void ParticleLayer::attachInteractor(vtkRenderWindowInteractor* interactor) {
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
    m_scalarBarWidget->SetInteractor(interactor);
    if (m_node->settings.showScalarBar && m_node->settings.isVisible) {
        m_scalarBarWidget->EnabledOn();
    }
}
void ParticleLayer::detachInteractor() {
    if (m_scalarBarWidget) {
        m_scalarBarWidget->EnabledOff();
        m_scalarBarWidget->SetInteractor(nullptr);
    }
}
void ParticleLayer::setVisible(bool visible) {
    // 1. Управляем видимостью основного актора
    if (m_actor) {
        m_actor->SetVisibility(visible ? 1 : 0);
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
bool ParticleLayer::isVisible() const {
    if (m_actor) {
        return m_actor->GetVisibility() != 0;
    }
    return false;
}
} // namespace QSpace::Visualize