#pragma once
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/VisualizeBaseEnums.h"
#include "Visualize/Layers/LayerSettings.h"
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
#include "FileSchemeStructures.h"
#include <memory>

namespace QSpace::Core {

// CRITICAL: полностью избавится от vtk
struct DataNode {
    QUuid                       id;     // уникальный идентификатор для связи между слоями и данными
    QString                     label;  // метка для отображения данных в UI
    QString                     path;   // путь к файлу с данными
    Visualize::EntityType       type;   // тип данных для быстрой подстройки визуализации
    vtkSmartPointer<vtkDataSet> data;   // непосредственно данные
    QSpace::IO::FileFormat      format; // формать файла (бинарный, текстовый и т.д.)
    QSpace::IO::ReadScheme      scheme; // схема для чтения данных, нужна для их восстановления)

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