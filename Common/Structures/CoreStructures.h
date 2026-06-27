#pragma once
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Structures/IOStructures.h"
#include <QMap>
#include <QPair>
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
#include "RenderStructures.h"
#include <memory>

namespace QSpace::Core {

struct VisualSettings {
    // Тип отрисовки: точки, размытые сферы (Splat) или объем
    Visualize::RenderMode mode      = Visualize::RenderMode::GausianSplat;
    bool                  isVisible = true;
    // геометрия
    double PointSize = 0.002; // размер частиц
    double opacity   = 0.99;  // непрозрачность
    // цветовая схема
    QString colorByField;
    QUuid   colorMapId;
    bool    useLogScale   = true;
    bool    showScalarBar = true;
    double  rangeMin      = 0.0;
    double  rangeMax      = 100.0;
    double  baseRangeMin;
    double  baseRangeMax;
    bool    autoRange              = true;
    bool    isEmmisive             = false;
    bool    hideOutOfRange         = true;
    double  exposureClamp          = 1.0;
    QString interpolationRangeType = Visualize::scalarBarRangeInterpolationTypeToString(
        Visualize::ScalarBarRangeInterpolation::Linear);
    QString ShaderType = Visualize::shaderTypeToString(Visualize::ShaderType::Default);
    QString interpolationOpacityFunction = Visualize::interpolationOpacityFunctionToString(
        Visualize::InterpolationOpacityFunction::Constant);

    double gaussianSharpness = 4.5; // Спад краев: 1.0 - пухлые сферы, 6.0 - резкие точки

    double sigmoidGammaOpacity = 6.0; // Крутизна перехода: больше = резче граница облака
    double sigmoidShiftOpacity = 0.2; // Сдвиг (0.0 - 1.0): отсекает фоновый "шум" мелких значений

    double sigmoidGammaColor = 6.0; // Крутизна перехода для цвета: больше = резче граница облака
    double sigmoidShiftColor =
        0.2; // Сдвиг для цвета (0.0 - 1.0): отсекает фоновый "шум" мелких значений
    double alpha =
        3.0; // Для Asinh: больше = сильнее выделяет мелкие значения, меньше = более линейная шкала

    VisualSettings() {
        // Забираем готовую Plasma. UUID будет сгенерирован автоматически внутри getPresetByName
        colorMapId = Visualize::ColorMapPresets::getPresetByName("Plasma").id;
    }

    // НОВОЕ: Метод для глубокого копирования (Сценарий 2: Уникальные настройки)
    std::shared_ptr<VisualSettings> clone() const {
        return std::make_shared<VisualSettings>(*this);
    }

    // НОВОЕ: Метод для частичного копирования (Сценарий 4: Копирование между слоями)
    void applyCompatible(const VisualSettings& source, Visualize::EntityType targetType) {
        // Общие параметры применяются всегда
        this->opacity       = source.opacity;
        this->isVisible     = source.isVisible;
        this->colorMapId    = source.colorMapId;
        this->useLogScale   = source.useLogScale;
        this->showScalarBar = source.showScalarBar;

        if (targetType == Visualize::EntityType::Gas) {
            this->mode = Visualize::RenderMode::GausianSplat;
        }
        // Специфичные параметры копируем с осторожностью
        // (Например, не применяем настройки GaussianSplat к Volume)
        if (this->mode == source.mode) {
            this->PointSize         = source.PointSize;
            this->gaussianSharpness = source.gaussianSharpness;
        }

        // Здесь можно добавить более сложную логику на основе targetType (Gas, Stars и т.д.)
    }
};

struct DataNode {
    QUuid                           id; // уникальный идентификатор для связи между слоями и данными
    QString                         label;  // метка для отображения данных в UI
    QString                         path;   // путь к файлу с данными
    Visualize::EntityType           type;   // тип данных для быстрой подстройки визуализации
    vtkSmartPointer<vtkDataSet>     data;   // непосредственно данные
    QSpace::IO::FileFormat          format; // формать файла (бинарный, текстовый и т.д.)
    QSpace::IO::ReadScheme          scheme; // схема для чтения данных, нужна для их восстановления)
    std::shared_ptr<VisualSettings> masterSettings;

    struct MetaData { // перенести вычисление метаданных в отдельный модуль physics
        double                               timestamp;
        double                               bounds[6];
        double                               center[3]       = {0, 0, 0};
        double                               centerOfMass[3] = {0, 0, 0};
        qint64                               pointCount;
        qint64                               cellCount;
        QMap<QString, QPair<double, double>> scalarRanges;
    } stats;

    DataNode(vtkSmartPointer<vtkDataSet> dataSet,
             const QString&              name,
             double                      timestamp = 0.0,
             Visualize::EntityType       t         = Visualize::EntityType::Unknown)
        : id(QUuid::createUuid()), label(name), type(t), data(dataSet) {
        masterSettings  = std::make_shared<VisualSettings>();
        stats.timestamp = timestamp;
    }
};

class Snapshot {
  public:
    QUuid                            id;
    QString                          name;
    double                           timestamp;
    QList<std::shared_ptr<DataNode>> components;

    Snapshot(const QString& snapshotName, double ts = 0.0)
        : id(QUuid::createUuid()), name(snapshotName), timestamp(ts) {
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
                mb->GetMetaData(i)->Set(vtkCompositeDataSet::NAME(),
                                        components[i]->label.toStdString().c_str());
            }
        }
        return mb;
    }
};

struct Experiment {
    QUuid                            id;
    QString                          name;
    QList<std::shared_ptr<Snapshot>> snapshots; // Список временных шагов

    Experiment(const QString& expName) : id(QUuid::createUuid()), name(expName) {
    }

    void addSnapshot(std::shared_ptr<Snapshot> snapshot) {
        if (snapshot)
            snapshots.append(snapshot);
    }
};

} // namespace QSpace::Core