#pragma once

#include "Common/Structures/SessionStructures.h"
#include "Structures/CoreStructures.h"
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <optional>


namespace QSpace::Session {

class ProjectSerializer {
  public:
    static QByteArray                                   serialize(const QSpace::Session::ProjectState& project_state);
    static std::optional<QSpace::Session::ProjectState> deserialize(const QByteArray& data);

  private:
    static QJsonObject                    serializeDataNode(const QSpace::Session::DataNodeState& node_state);
    static QSpace::Session::DataNodeState deserializeDataNode(const QJsonObject& json);

    static QJsonObject                  serializeVisualSettings(const QSpace::Core::VisualSettings& settings);
    static QSpace::Core::VisualSettings deserializeVisualSettings(const QJsonObject& json);

    static QJsonObject    serializeReadScheme(const IO::ReadScheme& scheme);
    static IO::ReadScheme deserializeReadScheme(const QJsonObject& json);

    // Вспомогательные методы для ColumnScheme
    static QJsonObject      serializeColumnScheme(const IO::ColumnScheme& cs);
    static IO::ColumnScheme deserializeColumnScheme(const QJsonObject& json);
};

} // namespace QSpace::Session