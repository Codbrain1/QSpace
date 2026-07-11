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

    vtkPointData* pointData = node->data->GetPointData();
    if (!pointData->HasArray(fieldName.toUtf8().constData()))
        return result;

    vtkDataArray* array = pointData->GetArray(fieldName.toUtf8().constData());
    if (!array)
        return result;

    const vtkIdType n       = array->GetNumberOfTuples();
    const int       numComp = array->GetNumberOfComponents(); // Смотрим, сколько компонент в поле

    result.reserve(int(n));

    if (numComp == 1) {
        // 1. Обычный скаляр (Масса, Плотность, Давление)
        for (vtkIdType i = 0; i < n; ++i) {
            result.append(static_cast<float>(array->GetTuple1(i)));
        }
    } else if (numComp == 3) {
        // 2. Векторное поле (Скорость / Velocity, Сила, Ускорение)
        // Выделяем на стеке небольшой буфер под 3 координаты
        double v[3] = {0.0, 0.0, 0.0};

        for (vtkIdType i = 0; i < n; ++i) {
            // Быстро вытаскиваем сразу весь вектор (Vx, Vy, Vz)
            array->GetTuple(i, v);

            // Вычисляем модуль скорости: sqrt(Vx^2 + Vy^2 + Vz^2)
            float speedMagnitude = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

            result.append(speedMagnitude);
        }
    } else {
        // 3. Резервный вариант (если компонент 2, 4 или больше)
        // Берем только первую компоненту, чтобы избежать паники VTK
        for (vtkIdType i = 0; i < n; ++i) {
            result.append(static_cast<float>(array->GetComponent(i, 0)));
        }
    }

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