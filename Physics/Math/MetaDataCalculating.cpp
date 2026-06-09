#include "MetaDataCalculating.h"

namespace QSpace::Physics::Math {
QSpace::Core::DataNode::MetaData calculateMetaData(vtkSmartPointer<vtkDataSet> dataSet,
                                                   double                      timestamp) {
    QSpace::Core::DataNode::MetaData stats;

    if (dataSet) {
        dataSet->GetBounds(stats.bounds);
        stats.pointCount = dataSet->GetNumberOfPoints();
        stats.cellCount  = dataSet->GetNumberOfCells();

        stats.center[0]  = (stats.bounds[0] + stats.bounds[1]) / 2.0;
        stats.center[1]  = (stats.bounds[2] + stats.bounds[3]) / 2.0;
        stats.center[2]  = (stats.bounds[4] + stats.bounds[5]) / 2.0;
        stats.timestamp  = timestamp;
        vtkPointData* pd = dataSet->GetPointData();
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
    return stats;
}
} // namespace QSpace::Physics::Math