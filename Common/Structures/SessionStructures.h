#pragma once
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Structures/IOStructures.h"
#include "CoreStructures.h"
#include <qlist.h>
#include <quuid.h>

namespace QSpace::Session {
struct DataNodeState {
    Core::VisualSettings     settings;
    QUuid                    id;
    QString                  label;
    QString                  path;
    Visualize::EntityType    type;
    Core::DataNode::MetaData stats;
    IO::FileFormat           format;
    IO::ReadScheme           scheme;
};
class DataContainerState { // TODO:: может оказаться излишним
};
struct ProjectState {
    QString              version = "1.0";
    QString              projectName;
    QList<DataNodeState> nodesStates;
    // TODO добавить список окон и их настройки
};
struct CurrentSession {
    QString projectName;
    QString projectFilePath;
    bool    isDirty = false;
};
} // namespace QSpace::Session