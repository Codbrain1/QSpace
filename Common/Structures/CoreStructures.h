#pragma once
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Structures/IOStructures.h"
#include "RenderStructures.h"
#include <QMap>
#include <QPair>
#include <memory>
#include <qcontainerfwd.h>
#include <qlist.h>
#include <qtypes.h>
#include <quuid.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataArray.h>
#include <vtkDataObjectTree.h>
#include <vtkDataSet.h>
#include <vtkInformation.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>

namespace QSpace::Core {

struct VisualSettings {
    // Тип отрисовки: точки, размытые сферы (Splat) или объем
    Visualize::RenderMode mode      = Visualize::RenderMode::GausianSplat;
    bool                  isVisible = true;
    // геометрия
    double PointSize = 0.002; // размер частиц
    double opacity   = 1.0;   // непрозрачность
    // цветовая схема
    QString colorByField;
    QUuid   colorMapId;
    bool    useLogScale   = true;
    bool    showScalarBar = true;
    double  rangeMin      = 0.0;
    double  rangeMax      = 100.0;
    double  baseRangeMin;
    double  baseRangeMax;
    bool    autoRange      = true;
    bool    isEmmisive     = false;
    bool    hideOutOfRange = true;
    double  exposureClamp  = 1.0;
    QString interpolationRangeType =
        Visualize::scalarBarRangeInterpolationTypeToString(Visualize::ScalarBarRangeInterpolation::Linear);
    QString ShaderType = Visualize::shaderTypeToString(Visualize::ShaderType::Default);
    QString interpolationOpacityFunction =
        Visualize::interpolationOpacityFunctionToString(Visualize::InterpolationOpacityFunction::Constant);

    double gaussianSharpness = 4.5; // Спад краев: 1.0 - пухлые сферы, 6.0 - резкие точки

    double sigmoidGammaOpacity = 6.0; // Крутизна перехода: больше = резче граница облака
    double sigmoidShiftOpacity = 0.2; // Сдвиг (0.0 - 1.0): отсекает фоновый "шум" мелких значений

    double sigmoidGammaColor = 6.0; // Крутизна перехода для цвета: больше = резче граница облака
    double sigmoidShiftColor = 0.2; // Сдвиг для цвета (0.0 - 1.0): отсекает фоновый "шум" мелких значений
    double alpha = 3.0; // Для Asinh: больше = сильнее выделяет мелкие значения, меньше = более линейная шкала
    VisualSettings() {
        // Забираем готовую Plasma. UUID будет сгенерирован автоматически внутри getPresetByName
        colorMapId = Visualize::ColorMapPresets::getPresetByName("Plasma").id;
    }
};
struct DataNode {
    QUuid                       id;
    QString                     label;
    QString                     path;
    Visualize::EntityType       type;
    vtkSmartPointer<vtkDataSet> data;
    QSpace::IO::FileFormat      format;
    QSpace::IO::ReadScheme      scheme;
    VisualSettings              settings;
    struct MetaData {
        double                               bounds[6];
        double                               centerOfMass[3] = {0, 0, 0};
        qint64                               pointCount;
        qint64                               cellCount;
        QMap<QString, QPair<double, double>> scalarRanges;
    } stats;
    DataNode(vtkSmartPointer<vtkDataSet> dataSet,
             const QString&              name,
             Visualize::EntityType       t = Visualize::EntityType::Unknown)
        : id(QUuid::createUuid()), label(name), type(t), data(dataSet) {
        if (data) {
            data->GetBounds(stats.bounds);
            stats.pointCount = data->GetNumberOfPoints();
            stats.cellCount  = data->GetNumberOfCells();

            stats.centerOfMass[0] = (stats.bounds[0] + stats.bounds[1]) / 2.0;
            stats.centerOfMass[1] = (stats.bounds[2] + stats.bounds[3]) / 2.0;
            stats.centerOfMass[2] = (stats.bounds[4] + stats.bounds[5]) / 2.0;

            vtkPointData* pd = data->GetPointData();
            if (pd) {
                for (int i = 0; i < pd->GetNumberOfArrays(); ++i) {
                    auto array = pd->GetArray(i);
                    if (array) {
                        double range[2];
                        array->GetRange(range);
                        stats.scalarRanges.insert(array->GetName(), {range[0], range[1]});
                    }
                }
            }
        }
    }
};

class DataContainer {
  public:
    QUuid                            id;
    QString                          name;
    QList<std::shared_ptr<DataNode>> components;
    DataContainer(const QString& containerName) : id(QUuid::createUuid()), name(containerName) {
    }
    void addComponent(std::shared_ptr<DataNode> node) {
        if (node) {
            components.append(node);
        }
    }
    vtkSmartPointer<vtkMultiBlockDataSet> asVtkMultiBlockDataSet() const {
        auto mb = vtkSmartPointer<vtkMultiBlockDataSet>::New();
        mb->SetNumberOfBlocks(components.size());
        for (int i = 0; i < components.size(); ++i) {
            if (components[i]->data) {
                mb->SetBlock(i, components[i]->data);
                mb->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(), components[i]->label.toStdString().c_str());
            }
        }
        return mb;
    }
};
} // namespace QSpace::Core