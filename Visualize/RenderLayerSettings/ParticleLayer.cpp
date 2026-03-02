#include "ParticleLayer.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Logger/Logger.h"
#include "Common/Structures/CoreStructures.h"
#include "Common/Structures/RenderStructures.h"
#include <cmath>
#include <memory>
#include <qloggingcategory.h>
#include <qstring.h>
#include <vtkAbstractArray.h>
#include <vtkActor.h>
#include <vtkCell.h>
#include <vtkColorTransferFunction.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkMappedDataArray.h>
#include <vtkPointData.h>
#include <vtkPointGaussianMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkType.h>

namespace QSpace::Visualize {
ParticleLayer::ParticleLayer(std::shared_ptr<Core::DataNode> node) : m_node(node) {
    m_mapper    = vtkSmartPointer<vtkPointGaussianMapper>::New();
    m_actor     = vtkSmartPointer<vtkActor>::New();
    m_lut       = vtkSmartPointer<vtkColorTransferFunction>::New();
    m_scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    m_mapper->SetEmissive(false);
    m_mapper->SetStatic(true);

    m_actor->SetMapper(m_mapper);
    m_node->settings.colorByField = m_node->data->GetPointData()->GetArrayName(0);
    // без этих строк видеокарта артефичит и экран мигает черным
    m_mapper->ScalarVisibilityOn();
    m_mapper->SetScalarModeToUsePointFieldData();
    m_lut->SetVectorModeToMagnitude();
    setupScalarBar();

    m_mapper->SetScalarModeToUsePointFieldData();
    m_mapper->SetColorModeToMapScalars();
    m_scalarBar->SetLookupTable(m_lut);
}
void ParticleLayer::update() {
    if (!m_node || !m_node->data) {
        qCCritical(LogRenderer) << "ParticleLayer::update() - Node or data is null";
        return;
    }

    // получаем данные
    auto data = vtkPolyData::SafeDownCast(m_node->data);
    if (!data || data->GetNumberOfPoints() == 0) {
        qCCritical(LogRenderer) << "node " + m_node->label + " don't contain particles. Particles count"
                                << (data ? data->GetNumberOfPoints() : -1);
        return;
    }

    // получаем настройки слоя
    auto& s = m_node->settings;

    if (m_mapper->GetInput() != data) {
        // задаем точки мапер для точек
        m_mapper->SetInputData(data);
    }

    if (s.mode == RenderMode::GausianSplat) {
        m_mapper->SetScaleFactor(s.PointSize);
        m_mapper->SetEmissive(true); // TODO: на белом фоне не отображаются частицы из за этой настройки
    } else if (s.mode == RenderMode::Points) {
        m_mapper->SetScaleFactor(0.000);
        m_actor->GetProperty()->SetPointSize(s.PointSize);
    } else {
        qCCritical(LogRenderer) << "Don't supported rendering mode: volume";
    }

    m_actor->GetProperty()->SetOpacity(s.opacity);
    m_actor->SetVisibility(s.isVisible);

    qCInfo(LogRenderer) << "Rendering" << data->GetNumberOfPoints() << "points. Bounds:" << data->GetBounds()[0]
                        << data->GetBounds()[1];

    if (!s.colorByField.isEmpty()) {
        vtkDataArray* arr = data->GetPointData()->GetArray(s.colorByField.toStdString().c_str());
        if (arr) {
            data->GetPointData()->SetActiveScalars(s.colorByField.toStdString().c_str());
            applyColorMap(s.colorMap);

            m_mapper->SelectColorArray(s.colorByField.toStdString().c_str());

            m_mapper->SetLookupTable(m_lut);

            double range[2] = {0.0, 1.0};
            if (s.autoRange) {
                // для отладки
                //  for (int k = 0; k < data->GetPointData()->GetNumberOfArrays(); ++k) {
                //      qCDebug(LogIO) << data->GetPointData()->GetArrayName(k);
                //  }
                // GetRange(-1) возвращает диапазон магнитуды вектора
                arr->GetRange(range, -1);
                // Защита от "нулевого" диапазона (если все скорости одинаковые)
                if (std::abs(range[1] - range[0]) < 1e-10) {
                    range[1] = range[0] + 1.0;
                }
            } else {
                range[0] = s.rangeMin;
                range[1] = s.rangeMax;
            }
            m_mapper->SetScalarRange(range);

            // обновление легенды
            m_scalarBar->SetTitle(s.colorByField.toStdString().c_str());
            m_scalarBar->SetVisibility(s.showScalarBar && s.isVisible);
        }

    } else {
        m_mapper->ScalarVisibilityOff();
        m_actor->GetProperty()->SetColor(1.0, 1.0, 1.0); // Белый по умолчанию
        m_scalarBar->SetVisibility(false);
    }
    m_mapper->Modified();
}
void ParticleLayer::applyColorMap(QSpace::Visualize::ColorMapType type) {
    m_lut->RemoveAllPoints();

    // получение точек
    auto points = QSpace::Visualize::ColorMapRegistry::getPresetPoints(type);
    for (const auto& pt : points) {
        m_lut->AddRGBPoint(pt.x, pt.r, pt.g, pt.b);
    }

    // Если в настройках включен логарифм (для космоса это часто нужно)
    if (m_node->settings.useLogScale) {
        m_lut->SetScaleToLog10();
    } else {
        m_lut->SetScaleToLinear();
    }
}
// заменяет указатель на данные (позволяет применять одинаковую настройку отображения для разных файлов)
// упрощает анимирование данных
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
    m_scalarBar->SetNumberOfLabels(5);
    m_scalarBar->SetWidth(0.1);  // 10% ширины экрана
    m_scalarBar->SetHeight(0.5); // 50% высоты экрана
    // Позиция справа
    m_scalarBar->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    m_scalarBar->GetPositionCoordinate()->SetValue(0.85, 0.05);

    // Настройка текста
    vtkTextProperty* txt = m_scalarBar->GetLabelTextProperty();
    txt->SetColor(1.0, 1.0, 1.0); // Белый текст
    txt->SetFontSize(12);
    m_scalarBar->GetTitleTextProperty()->SetColor(1.0, 1.0, 1.0);
}
} // namespace QSpace::Visualize