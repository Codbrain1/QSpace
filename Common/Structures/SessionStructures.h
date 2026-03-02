#pragma once
#include "Common/Enums/RenderEnums.h"
#include "CoreStructures.h"
#include <quuid.h>
namespace QSpace::Session {
class DataNodeState {
    Core::VisualSettings     settings;
    QUuid                    id;
    QString                  label;
    Visualize::EntityType    type;
    Core::DataNode::MetaData stats;
};
class DataContainerState {};
class ProjectState {};
} // namespace QSpace::Session