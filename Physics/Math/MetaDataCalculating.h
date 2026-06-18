#pragma once
#include "Common/Structures/CoreStructures.h"

namespace QSpace::Physics::Math {
QSpace::Core::DataNode::MetaData calculateMetaData(vtkSmartPointer<vtkDataSet> dataSet,
                                                   double                      timestamp);
double                           calculateTimestamp(double timestamp);
} // namespace QSpace::Physics::Math