#pragma once
#include "Common/Structures/ObjectRegistryStructures.h"
#include <QVector3D>
#include <QVector>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <utility>

// TODO временный адаптер для vtk массивов
namespace QSpace::Visualize::vtkAdapter {

// vtkDataSet сам по себе не гарантирует наличие точек (например, vtkImageData),
// поэтому кастуем к vtkPointSet — базовому классу для vtkPolyData/vtkUnstructuredGrid,
// которые реально хранят частицы как точки.
inline QVector<QVector3D> extractPositions(const std::shared_ptr<Core::DataNode>& node) {
    QVector<QVector3D> result;
    if (!node || !node->data)
        return result;

    vtkPointSet* pointSet = vtkPointSet::SafeDownCast(node->data);
    if (!pointSet || !pointSet->GetPoints())
        return result;

    vtkPoints*      points = pointSet->GetPoints();
    const vtkIdType n      = points->GetNumberOfPoints();
    result.reserve(int(n));

    for (vtkIdType i = 0; i < n; ++i) {
        double p[3];
        points->GetPoint(i, p);
        result.append(QVector3D(float(p[0]), float(p[1]), float(p[2])));
    }
    return result;
}

inline QVector<float> extractScalarField(const std::shared_ptr<Core::DataNode>& node,
                                         const QString&                         fieldName) {
    QVector<float> result;
    if (!node || !node->data || fieldName.isEmpty())
        return result;

    vtkDataArray* array = node->data->GetPointData()->GetArray(fieldName.toUtf8().constData());
    if (!array)
        return result;

    const vtkIdType n = array->GetNumberOfTuples();
    result.reserve(int(n));
    for (vtkIdType i = 0; i < n; ++i)
        result.append(float(array->GetTuple1(i)));

    return result;
}

inline std::pair<double, double> getScalarRange(const std::shared_ptr<Core::DataNode>& node,
                                                const QString&                         fieldName) {
    if (!node || !node->data || fieldName.isEmpty())
        return std::pair(0.0, 0.0);
    vtkDataArray* array = node->data->GetPointData()->GetArray(fieldName.toUtf8().constData());
    if (!array)
        return std::pair(0.0, 0.0);
    double range[2];
    array->GetRange(range);
    return std::pair(range[0], range[1]);
}

// список имён доступных скалярных полей — нужен для UI (выбор colorByField)
inline QStringList availableScalarFields(const std::shared_ptr<Core::DataNode>& node) {
    QStringList names;
    if (!node || !node->data)
        return names;

    vtkPointData* pd = node->data->GetPointData();
    for (int i = 0; i < pd->GetNumberOfArrays(); ++i)
        names << QString::fromUtf8(pd->GetArrayName(i));
    return names;
}

} // namespace QSpace::Visualize::vtkAdapter