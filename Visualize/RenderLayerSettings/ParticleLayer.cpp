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
#include <vtkImageData.h>
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

    m_mapper->SetEmissive(false);
    m_mapper->SetStatic(true);

    m_actor->SetMapper(m_mapper);
    m_node->settings.colorByField = m_node->data->GetPointData()->GetArrayName(0);
    // без этих строк видеокарта артефичит и экран мигает черным
    m_mapper->ScalarVisibilityOn();
    m_mapper->SetScalarModeToUsePointFieldData();
    m_mapper->SetScalarModeToUsePointData();
    m_lut->SetVectorModeToMagnitude();

    setupScalarBar();

    m_mapper->SetScalarModeToUsePointFieldData();
    m_scalarBar->SetLookupTable(m_lut);
    m_mapper->SetColorModeToMapScalars();
    m_mapper->ScalarVisibilityOn();
    m_scalarBar->SetUseOpacity(true);
    m_actor->PickableOn();
    // m_actor->GetMapper()->SetInterpolateScalarsBeforeMapping(true);
    // m_actor->GetProperty()->SetAmbient(1.0);
    // m_actor->GetProperty()->SetDiffuse(0.0);
    // m_actor->GetProperty()->SetSpecular(0.0);
}
void ParticleLayer::update() {
    if (!m_node || !m_node->data) {
        qCCritical(LogRenderer) << "ParticleLayer::update() - Node or data is null";
        return;
    }

    auto data = vtkPolyData::SafeDownCast(m_node->data);
    if (!data || data->GetNumberOfPoints() == 0) {
        qCCritical(LogRenderer) << "Node" << m_node->label << "doesn't contain particles.";
        return;
    }

    auto& s = m_node->settings;
    m_actor->SetVisibility(s.isVisible);

    if (m_mapper->GetInput() != data) {
        m_mapper->SetInputData(data);
    }
    // 1. Применяем базовые настройки рендера
    applyRenderModeSettings(s);

    // 2. Настраиваем данные для раскраски
    setupDataArrays(data, s);

    // 3. Обновляем видимость интерфейса
    updateScalarBarVisibility(s);

    // 4. Обновляем шейдер
    if (s.mode == RenderMode::GausianSplat) {
        updateShader(s);
    }
    m_mapper->Modified();
}

void ParticleLayer::applyRenderModeSettings(const Core::VisualSettings& s) {
    if (s.mode == RenderMode::GausianSplat) {
        m_mapper->SetScaleFactor(s.PointSize);
        m_mapper->SetScalarOpacityFunction(m_opacityFunction);
        m_mapper->SetColorModeToMapScalars();
        m_mapper->SetScalarModeToUsePointFieldData();
        m_mapper->SetScalarModeToUsePointData();
        m_mapper->SetEmissive(s.isEmmisive);
    } else if (s.mode == RenderMode::Points) {
        m_mapper->SetScaleFactor(0.0);
        m_actor->GetProperty()->SetPointSize(s.PointSize);
    } else {
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
        m_scalarBar->SetVisibility(false);
        return;
    }
    std::string sourceFieldName = s.colorByField.toStdString();
    std::string targetFieldName = sourceFieldName;
    if (s.useLogScale) {
        targetFieldName = "Log_" + sourceFieldName;

        // Проверяем, не вычисляли ли мы логарифм для этого поля ранее
        if (!data->GetPointData()->HasArray(targetFieldName.c_str())) {
            vtkSmartPointer<vtkArrayCalculator> calc = vtkSmartPointer<vtkArrayCalculator>::New();
            calc->SetInputData(data);

            // Переменная "v" будет представлять текущее выбранное поле
            calc->AddScalarVariable("v", sourceFieldName.c_str(), 0);

            // Формула с предохранителем от нуля. 1e-10 можно вынести в настройки.
            calc->SetFunction("log10(abs(v)+ 1e-10)");
            calc->SetResultArrayName(targetFieldName.c_str());
            calc->Update();

            // Добавляем результат обратно в исходные данные, чтобы не считать каждый раз
            vtkDataArray* logArr = calc->GetDataSetOutput()->GetPointData()->GetArray(targetFieldName.c_str());
            data->GetPointData()->AddArray(logArr);
        }
    }
    vtkDataArray* arr = data->GetPointData()->GetArray(targetFieldName.c_str());
    if (!arr)
        return;

    int comp = arr->GetNumberOfComponents() == 3 ? -1 : 0;
    data->GetPointData()->SetActiveScalars(targetFieldName.c_str());

    m_mapper->SelectColorArray(targetFieldName.c_str());
    m_mapper->SetOpacityArray(targetFieldName.c_str());
    m_mapper->SetArrayComponent(comp);
    m_mapper->SetOpacityArrayComponent(comp);
    m_mapper->SetLookupTable(m_lut);

    double range[2] = {0.0, 1.0};

    s.baseRangeMin = range[0];
    s.baseRangeMax = range[1];

    if (s.autoRange) {
        s.rangeMin = range[0];
        s.rangeMax = range[1]; // TODO: исправить работу авто-диапазона
    } else {
        range[0] = s.rangeMin;
        range[1] = s.rangeMax;
    }

    applyColorMap(s.colorMapId, range);
    m_mapper->SetScalarRange(range);
}
void ParticleLayer::applyColorMap(QUuid& colorMapUuid, double range[2]) {
    m_lut->RemoveAllPoints();
    m_opacityFunction->RemoveAllPoints();
    m_opacityFunction->ClampingOn();
    m_lut->SetScaleToLinear();
    m_lut->SetColorSpaceToLab();

    double minVal = range[0];
    double maxVal = range[1];

    m_lut->SetUseBelowRangeColor(false);
    m_lut->SetUseAboveRangeColor(false);

    auto& colorMapManager = QSpace::Visualize::ColorMapManager::instance();
    auto  colorMap        = colorMapManager.getMap(colorMapUuid);
    if (!colorMap.has_value()) {
        colorMap = Visualize::ColorMapPresets::getStandardPresets().first();
    }

    vtkSmartPointer<vtkColorTransferFunction> tempLut = vtkSmartPointer<vtkColorTransferFunction>::New();
    tempLut->SetColorSpaceToLab();
    for (const auto& pt : colorMap->points) {
        tempLut->AddRGBPoint(pt.x, pt.r, pt.g, pt.b);
    }

    const int numSamples = 256;

    double alpha           = m_node->settings.alpha;
    auto interpolationType = Visualize::scalarBarRangeInterpolationFromString(m_node->settings.interpolationRangeType);
    auto opacityInterpolationType =
        Visualize::interpolationOpacityFunctionFromString(m_node->settings.interpolationOpacityFunction);
    for (int i = 0; i < numSamples; ++i) {
        double t_step = static_cast<double>(i) / (numSamples - 1);
        // Линейно распределенные значения
        double val = minVal + t_step * (maxVal - minVal);

        // Применяем твои кастомные функции сглаживания к нормализованному шагу
        if (interpolationType == Visualize::ScalarBarRangeInterpolation::Sigmoid) {
            double gamma  = m_node->settings.sigmoidGammaColor;
            double shift  = m_node->settings.sigmoidShiftColor;
            double minSig = 1.0 / (1.0 + std::exp(gamma * shift));
            double maxSig = 1.0 / (1.0 + std::exp(-gamma * (1.0 - shift)));
            double s_val  = 1.0 / (1.0 + std::exp(-gamma * (t_step - shift)));
            t_step        = (s_val - minSig) / (maxSig - minSig + 1e-9);
        } else if (interpolationType == Visualize::ScalarBarRangeInterpolation::Asinh) {
            t_step = std::asinh(t_step * alpha) / std::asinh(alpha);
        }

        t_step = std::clamp(t_step, 0.0, 1.0);

        double color[3];
        tempLut->GetColor(t_step, color);

        double baseOpacity = m_node->settings.opacity;

        if (opacityInterpolationType == Visualize::InterpolationOpacityFunction::Sigmoid) {
            double gamma  = m_node->settings.sigmoidGammaOpacity;
            double shift  = m_node->settings.sigmoidShiftOpacity;
            double minSig = 1.0 / (1.0 + std::exp(gamma * shift));
            double maxSig = 1.0 / (1.0 + std::exp(-gamma * (1.0 - shift)));
            double s_val  = 1.0 / (1.0 + std::exp(-gamma * (t_step - shift)));
            baseOpacity   = baseOpacity * ((s_val - minSig) / (maxSig - minSig + 1e-9));
        } else if (opacityInterpolationType == Visualize::InterpolationOpacityFunction::Asinh) {
            double asinhFactor = std::asinh(t_step * baseOpacity) / std::asinh(baseOpacity);
            baseOpacity        = baseOpacity * asinhFactor;
        } else if (opacityInterpolationType == Visualize::InterpolationOpacityFunction::Linear) {
            baseOpacity = baseOpacity * t_step;
        } else if (opacityInterpolationType == Visualize::InterpolationOpacityFunction::Quadro) {
            baseOpacity = baseOpacity * t_step * t_step;
        } else if (opacityInterpolationType == Visualize::InterpolationOpacityFunction::Qube) {
            baseOpacity = baseOpacity * t_step * t_step * t_step;
        } else if (opacityInterpolationType == Visualize::InterpolationOpacityFunction::Sqrt) {
            baseOpacity = baseOpacity * std::sqrt(t_step);
        }
        baseOpacity = std::clamp(baseOpacity, 0.0, 1.0);
        m_opacityFunction->AddPoint(val, baseOpacity);
        m_lut->AddRGBPoint(val, color[0], color[1], color[2]);
    }
    if (m_node->settings.hideOutOfRange) {
        double safeEps = (maxVal - minVal) * 1e-3;
        m_opacityFunction->AddPoint(minVal - safeEps, 0.0);
        m_opacityFunction->AddPoint(minVal,
                                    (opacityInterpolationType == Visualize::InterpolationOpacityFunction::Constant)
                                        ? m_node->settings.opacity
                                        : 0.0);

        m_opacityFunction->AddPoint(maxVal, m_node->settings.opacity);
        m_opacityFunction->AddPoint(maxVal + safeEps, 0.0);
    }
}

void ParticleLayer::updateShader(const Core::VisualSettings& s) {
    auto shader = Visualize::shaderTypeFromString(s.ShaderType);
    if (shader == ShaderType::Default) {
        m_mapper->SetSplatShaderCode(nullptr);
    } else {
        QString shaderCode = QString("//VTK::Color::Impl\n"
                                     "float dist2 = dot(offsetVCVSOutput.xy, offsetVCVSOutput.xy);\n"
                                     "if (dist2 > 1.0) discard;\n"

                                     "float gaussian = exp(-%1 * dist2);\n"
                                     "opacity = opacity * gaussian;\n"

                                     "vec3 baseColor = vertexColorVSOutput.rgb;\n"
                                     "if (%2) {\n" // Режим Emissive
                                     "    diffuseColor = vec3(0.0);\n"
                                     "    // ВАЖНО: Чтобы центр был желтым, мы ограничиваем вклад одной частицы.\n"
                                     "    ambientColor = baseColor * opacity * %3;\n"
                                     "} else {\n" // Обычный режим
                                     "    diffuseColor = baseColor;\n"
                                     "    ambientColor = baseColor * opacity*0.9;\n"
                                     "}\n")
                                 .arg(s.gaussianSharpness)
                                 .arg(s.isEmmisive ? "true" : "false")
                                 .arg(s.exposureClamp);
        m_mapper->SetSplatShaderCode(shaderCode.toUtf8().constData());
    }
    m_actor->GetMapper()->Modified();
    m_mapper->Modified();
}
void ParticleLayer::updateScalarBarVisibility(const Core::VisualSettings& s) {
    bool shouldShow = s.showScalarBar && s.isVisible;
    m_scalarBar->SetVisibility(shouldShow);

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

        int numTicks = m_scalarBar->GetNumberOfLabels();
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
    txt->SetColor(0, 0, 0); // Черный текст
    txt->SetFontSize(16);
    m_scalarBar->GetTitleTextProperty()->SetColor(0, 0, 0);
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