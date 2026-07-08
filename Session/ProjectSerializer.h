#pragma once

#include "Common/Structures/SessionStructures.h"
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "Structures/ObjectRegistryStructures.h"
#include <optional>

namespace QSpace::Session {

class ProjectSerializer {
  public:
    static QByteArray serialize(const QSpace::Session::ProjectState& project_state);
    static std::optional<QSpace::Session::ProjectState> deserialize(const QByteArray& data);

    static QJsonObject                 serializeColorMap(const QSpace::Visualize::ColorMap& map);
    static QSpace::Visualize::ColorMap deserializeColorMap(const QJsonObject& json);

  private:
    static QJsonObject serializeDataNode(const QSpace::Session::DataNodeState& node_state);
    static QSpace::Session::DataNodeState deserializeDataNode(const QJsonObject& json);

    static QJsonObject
    serializeVisualSettings(const QSpace::Visualize::Layers::LayerSettings& settings);
    static std::shared_ptr<QSpace::Visualize::Layers::LayerSettings>
    deserializeVisualSettings(const QJsonObject& json);

    static QJsonObject    serializeReadScheme(const IO::ReadScheme& scheme);
    static IO::ReadScheme deserializeReadScheme(const QJsonObject& json);

    // Вспомогательные методы для ColumnScheme
    static QJsonObject      serializeColumnScheme(const IO::ColumnScheme& cs);
    static IO::ColumnScheme deserializeColumnScheme(const QJsonObject& json);
};

} // namespace QSpace::Session