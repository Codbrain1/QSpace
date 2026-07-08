#include "ParticleLayer.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Logger/Logger.h"
#include "Common/Structures/CoreStructures.h"
#include <qloggingcategory.h>
#include <qnamespace.h>
#include <qstring.h>
#include <quuid.h>
#include <vtkAbstractArray.h>
#include <vtkActor.h>
#include <vtkCell.h>
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
#include "BaseLayer.h"
#include <cmath>
#include <memory>

namespace QSpace::Visualize {
ParticleLayer::ParticleLayer(std::shared_ptr<Core::DataNode> node) : BaseLayer(node) {
    m_mapper = vtkSmartPointer<vtkPointGaussianMapper>::New();
    m_actor  = vtkSmartPointer<vtkActor>::New();
    // Инициализация пайплайна
    m_actor->SetMapper(m_mapper);
    // m_actor->ForceTranslucentOn();
    m_actor->GetProperty()->SetLighting(false);
    m_mapper->SetStatic(true);
    m_mapper->SetEmissive(false);

    m_actor->SetDragable(false);

    // Настройка маппинга
    m_mapper->ScalarVisibilityOn();
    m_mapper->SetScalarModeToUsePointFieldData();
    m_mapper->SetColorModeToMapScalars();
    m_mapper->SetLookupTable(m_lut);
    m_actor->PickableOn();
    m_baseProp   = m_actor;
    m_baseMapper = m_mapper;
}

void ParticleLayer::applyRenderModeSettings(const Visualize::Layers::LayerSettings& s) {
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

// void ParticleLayer::setupDataArrays(Visualize::Layers::LayerSettings& s) {
//     if (s.colorByField.isEmpty()) {
//         m_mapper->ScalarVisibilityOff();
//         m_actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
//         return;
//     }
//     std::string sourceField         = s.colorByField.toStdString();
//     std::string currentWorkingField = sourceField;
//     auto        lockedNode          = m_node.lock();
//     if (!lockedNode)
//         return;
//     auto data = vtkPolyData::SafeDownCast(lockedNode->data);
//     if (!data)
//         return;

//     if (s.colorByField.contains("energy", Qt::CaseInsensitive)) {
//         std::string dimField = "Dimension_" + sourceField;
//         if (!data->GetPointData()->HasArray(dimField.c_str())) {
//             auto calc = vtkSmartPointer<vtkArrayCalculator>::New();
//             calc->SetInputData(data);
//             calc->AddScalarVariable("v", sourceField.c_str(), 0);
//             // Сохранена оригинальная формула SPH интерполяции
//             calc->SetFunction("sqrt(1.66666666667 * 0.66666666667*v)^2 * 1.79E+6");
//             calc->SetResultArrayName(dimField.c_str());
//             calc->Update();
//             data->GetPointData()->AddArray(
//                 calc->GetDataSetOutput()->GetPointData()->GetArray(dimField.c_str()));
//         }
//         currentWorkingField = dimField;
//     }
//     vtkDataArray* arr_temp = data->GetPointData()->GetArray(currentWorkingField.c_str());
//     int           comp     = (arr_temp->GetNumberOfComponents() == 3) ? -1 : 0;
//     double        range[2] = {0.0, 1.0};
//     arr_temp->GetRange(range, comp);

//     // устанавливаем базовый диапазон для возможности его использования при авто-диапазоне и в UI
//     s.baseRangeMin         = range[0];
//     s.baseRangeMax         = range[1];
//     std::string finalField = currentWorkingField;
//     if (s.useLogScale) {
//         finalField = "Log_" + currentWorkingField;
//         if (!data->GetPointData()->HasArray(finalField.c_str())) {
//             auto calc = vtkSmartPointer<vtkArrayCalculator>::New();
//             calc->SetInputData(data);
//             calc->AddScalarVariable("v", currentWorkingField.c_str(), 0);
//             calc->SetFunction("log10(abs(v) + 1e-10)");
//             calc->SetResultArrayName(finalField.c_str());
//             calc->Update();
//             data->GetPointData()->AddArray(
//                 calc->GetDataSetOutput()->GetPointData()->GetArray(finalField.c_str()));
//         }
//     }
//     vtkDataArray* arr = data->GetPointData()->GetArray(finalField.c_str());
//     if (!arr)
//         return;
//     data->GetPointData()->SetActiveScalars(finalField.c_str());
//     m_mapper->SelectColorArray(finalField.c_str());
//     m_mapper->SetOpacityArray(finalField.c_str());
//     m_mapper->SetArrayComponent(comp);
//     m_mapper->SetOpacityArrayComponent(comp);
//     m_mapper->SetLookupTable(m_lut);

//     if (s.autoRange) {
//         arr->GetRange(range, comp);
//         s.rangeMin = s.baseRangeMin;
//         s.rangeMax = s.baseRangeMax;
//     } else {
//         range[0] = s.rangeMin;
//         range[1] = s.rangeMax;
//         if (s.useLogScale) {
//             range[0] = std::log10(std::abs(s.rangeMin) + 1e-10);
//             range[1] = std::log10(std::abs(s.rangeMax) + 1e-10);
//         }
//     }
//     applyColorMap(s.colorMapId, range);

//     m_mapper->SetScalarRange(range);
// }

void ParticleLayer::setupDataArrays(Visualize::Layers::LayerSettings& s) {
    if (s.colorByField.isEmpty()) {
        m_mapper->ScalarVisibilityOff();
        m_actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
        return;
    }

    std::string sourceField         = s.colorByField.toStdString();
    std::string currentWorkingField = sourceField;

    auto lockedNode = m_node.lock();
    if (!lockedNode)
        return;

    auto data = vtkPolyData::SafeDownCast(lockedNode->data);

    if (!data)
        return;

    // --- 1. Вычисление энергии (если нужно) ---
    if (s.colorByField.contains("energy", Qt::CaseInsensitive)) {
        std::string dimField = "Dimension_" + sourceField;
        if (!data->GetPointData()->HasArray(dimField.c_str())) {
            auto calc = vtkSmartPointer<vtkArrayCalculator>::New();
            calc->SetInputData(data);
            calc->AddScalarVariable("v", sourceField.c_str(), 0);
            calc->SetFunction("sqrt(1.66666666667 * 0.66666666667*v)^2 * 1.79E+6");
            calc->SetResultArrayName(dimField.c_str());
            calc->Update();
            data->GetPointData()->AddArray(
                calc->GetDataSetOutput()->GetPointData()->GetArray(dimField.c_str()));
        }
        currentWorkingField = dimField;
    }

    // Получаем базовый массив для расчетов диапазонов
    vtkDataArray* arr_temp = data->GetPointData()->GetArray(currentWorkingField.c_str());
    if (!arr_temp)
        return; // Защита от nullptr

    int    comp     = (arr_temp->GetNumberOfComponents() == 3) ? -1 : 0;
    double range[2] = {0.0, 1.0};

    // ВНИМАНИЕ: GetRange может быть дорогим, если массив только что создан,
    // но VTK его кэширует после первого вызова.
    arr_temp->GetRange(range, comp);

    s.baseRangeMin = range[0];
    s.baseRangeMax = range[1];

    std::string finalField = currentWorkingField;

    // --- 2. Логарифмирование ---
    if (s.useLogScale) {
        finalField = "Log_" + currentWorkingField;
        if (!data->GetPointData()->HasArray(finalField.c_str())) {
            auto calc = vtkSmartPointer<vtkArrayCalculator>::New();
            calc->SetInputData(data);
            calc->AddScalarVariable("v", currentWorkingField.c_str(), 0);
            calc->SetFunction("log10(abs(v) + 1e-10)");
            calc->SetResultArrayName(finalField.c_str());
            calc->Update();
            data->GetPointData()->AddArray(
                calc->GetDataSetOutput()->GetPointData()->GetArray(finalField.c_str()));
        }
    }

    vtkDataArray* arr = data->GetPointData()->GetArray(finalField.c_str());
    if (!arr)
        return;

    // --- 3. Настройка Маппера (БЕЗ модификации vtkPolyData) ---

    // УДАЛЕНО: data->GetPointData()->SetActiveScalars(finalField.c_str());

    // Используем только API маппера:
    m_mapper->ScalarVisibilityOn();
    m_mapper->SelectColorArray(finalField.c_str());
    m_mapper->SetOpacityArray(finalField.c_str());
    m_mapper->SetArrayComponent(comp);
    m_mapper->SetOpacityArrayComponent(comp);

    // УДАЛЕНО: m_mapper->SetLookupTable(m_lut);
    // Это нужно перенести в конструктор ParticleLayer или BaseLayer::setupLayer().
    // Нет нужды переназначать указатель каждый кадр.

    // --- 4. Расчет диапазонов ---
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

    // --- 5. Обновление цветов ---

    applyColorMap(s.colorMapId, range);
    m_mapper->SetScalarRange(range);
}

void ParticleLayer::updateShader(const Visualize::Layers::LayerSettings& s) {
    // 1. Обработка дефолтного шейдера
    if (shaderTypeFromString(s.ShaderType) == ShaderType::Default) {
        // Сбрасываем только если там сейчас не nullptr
        if (m_mapper->GetSplatShaderCode() != nullptr) {
            m_mapper->SetSplatShaderCode(nullptr);
        }
        return;
    }

    // 2. Генерация нового кода шейдера на основе текущих настроек
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

    // 3. Проверяем текущий код в маппере
    const char* currentCode = m_mapper->GetSplatShaderCode();

    // Если код не был установлен ранее ИЛИ он отличается от нового, обновляем
    if (!currentCode || shaderCode != currentCode) {
        m_mapper->SetSplatShaderCode(shaderCode.toUtf8().constData());
    }
}
} // namespace QSpace::Visualize