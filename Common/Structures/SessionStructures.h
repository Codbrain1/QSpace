#pragma once
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Structures/FileSchemeStructures.h"
#include <qlist.h>
#include <quuid.h>
#include "ObjectRegistryStructures.h"

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

struct LayerState {
    QUuid                layerId;
    QUuid                nodeId;   // На какой DataNode ссылается
    QUuid                viewId;   // В каком окне отрисовывается
    Core::VisualSettings settings; // Настройки ИМЕННО ЭТОГО слоя
};

class SnapshotState { // TODO:: может оказаться излишним
};

struct ProjectState {
    QString              version = "1.0";
    QString              projectName;
    QList<DataNodeState> nodesStates;
    QList<LayerState>    layersStates;
};

struct CurrentSession {
    QString projectName;
    QString projectFilePath;
    bool    isDirty = false;
};
} // namespace QSpace::Session