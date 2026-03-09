#include "ProjectSerializer.h"
#include "Enums/IOEnums.h"
#include "Enums/RenderEnums.h"
#include <QVariant>

namespace QSpace::Session {

// =========================================================================
// ОСНОВНЫЕ МЕТОДЫ ПРОЕКТА
// =========================================================================

QByteArray ProjectSerializer::serialize(const QSpace::Session::ProjectState& project_state) {
    QJsonObject root;
    root.insert("version", project_state.version);

    QJsonArray arr;
    for (const auto& node_state : project_state.nodesStates) {
        // Теперь мы просто вызываем выделенный метод
        arr.append(serializeDataNode(node_state));
    }
    root.insert("nodes", arr);

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}

std::optional<ProjectState> ProjectSerializer::deserialize(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return std::nullopt;
    }

    QJsonObject  root = doc.object();
    ProjectState state;
    state.version = root["version"].toString("1.0");

    QJsonArray nodesArr = root["nodes"].toArray();
    for (const auto& nodeVal : nodesArr) {
        if (nodeVal.isObject()) {
            state.nodesStates.append(deserializeDataNode(nodeVal.toObject()));
        }
    }

    return state;
}

// =========================================================================
// СЕРИАЛИЗАЦИЯ НОДЫ (ИСПРАВЛЕНА ОШИБКА)
// =========================================================================

QJsonObject ProjectSerializer::serializeDataNode(const QSpace::Session::DataNodeState& node_state) {
    QJsonObject o;
    o.insert("id", node_state.id.toString());
    o.insert("label", node_state.label);
    o.insert("path", node_state.path);
    o.insert("format", IO::fileformatToString(node_state.format)); // Проверь: ...ToString или ...String в твоих енамах
    o.insert("entityType", QSpace::Visualize::entitytypeToString(node_state.type));
    // Bounds
    QJsonArray boundsArr;
    for (int i = 0; i < 6; ++i)
        boundsArr.append(node_state.stats.bounds[i]);
    o.insert("bounds", boundsArr);

    // Counts (приводим к qint64 явно, чтобы избежать предупреждений)
    o.insert("cellCount", static_cast<qint64>(node_state.stats.cellCount));
    o.insert("pointCount", static_cast<qint64>(node_state.stats.pointCount));

    // CenterOfMass
    QJsonArray centerOfMassArr;
    for (int i = 0; i < 3; ++i)
        centerOfMassArr.append(node_state.stats.centerOfMass[i]);
    o.insert("centerOfMass", centerOfMassArr);

    // ScalarRanges
    QJsonObject rangesObj;
    for (const auto& [key, range] : node_state.stats.scalarRanges.asKeyValueRange()) {
        QJsonObject r;
        r.insert("min", range.first);
        r.insert("max", range.second);
        rangesObj.insert(key, r);
    }
    o.insert("scalarRanges", rangesObj);

    // Вложенные структуры
    o.insert("visualSettings", serializeVisualSettings(node_state.settings));
    o.insert("readScheme", serializeReadScheme(node_state.scheme));

    return o;
}

DataNodeState ProjectSerializer::deserializeDataNode(const QJsonObject& json) {
    DataNodeState node;

    node.id    = QUuid::fromString(json["id"].toString());
    node.label = json["label"].toString();
    node.path  = json["path"].toString();

    node.type   = Visualize::entitytypeFromString(json["entityType"].toString());
    node.format = IO::fileformatFromString(json["format"].toString());

    QJsonArray bArr = json["bounds"].toArray();
    for (int i = 0; i < 6 && i < bArr.size(); ++i) {
        node.stats.bounds[i] = bArr[i].toDouble();
    }

    node.stats.cellCount  = json["cellCount"].toVariant().toLongLong();
    node.stats.pointCount = json["pointCount"].toVariant().toLongLong();

    QJsonArray comArr = json["centerOfMass"].toArray();
    for (int i = 0; i < 3 && i < comArr.size(); ++i) {
        node.stats.centerOfMass[i] = comArr[i].toDouble();
    }

    QJsonObject rangesObj = json["scalarRanges"].toObject();
    for (auto it = rangesObj.begin(); it != rangesObj.end(); ++it) {
        QJsonObject r = it.value().toObject();
        node.stats.scalarRanges.insert(it.key(), {r["min"].toDouble(), r["max"].toDouble()});
    }

    if (json.contains("visualSettings")) {
        node.settings = deserializeVisualSettings(json["visualSettings"].toObject());
    }
    if (json.contains("readScheme")) {
        node.scheme = deserializeReadScheme(json["readScheme"].toObject());
    }

    return node;
}

// =========================================================================
// ВИЗУАЛЬНЫЕ НАСТРОЙКИ
// =========================================================================

QJsonObject ProjectSerializer::serializeVisualSettings(const QSpace::Core::VisualSettings& settings) {
    QJsonObject obj;
    obj.insert("mode", QSpace::Visualize::rendermodeToString(settings.mode));
    obj.insert("colorMap", QSpace::Visualize::colormapToString(settings.colorMap));
    obj.insert("isVisible", settings.isVisible);
    obj.insert("useLogScale", settings.useLogScale);
    obj.insert("showScalarBar", settings.showScalarBar);
    obj.insert("autoRange", settings.autoRange);
    obj.insert("pointSize", settings.PointSize);
    obj.insert("opacity", settings.opacity);
    obj.insert("alpha", settings.alpha);
    obj.insert("beta", settings.beta);
    obj.insert("rangeMin", settings.rangeMin);
    obj.insert("rangeMax", settings.rangeMax);
    obj.insert("colorByField", settings.colorByField);
    obj.insert("isEmisive", settings.isEmmisive);
    return obj;
}

QSpace::Core::VisualSettings ProjectSerializer::deserializeVisualSettings(const QJsonObject& json) {
    QSpace::Core::VisualSettings vs;
    vs.mode     = Visualize::rendermodeFromString(json["mode"].toString()).value_or(Visualize::RenderMode::Points);
    vs.colorMap = Visualize::colormapFromString(json["colorMap"].toString()).value_or(Visualize::ColorMapType::Viridis);
    vs.isVisible     = json["isVisible"].toBool(true);
    vs.PointSize     = json["pointSize"].toDouble(0.005);
    vs.opacity       = json["opacity"].toDouble(1.0);
    vs.alpha         = json["alpha"].toDouble(0.0);
    vs.beta          = json["beta"].toDouble(0.0);
    vs.colorByField  = json["colorByField"].toString();
    vs.useLogScale   = json["useLogScale"].toBool(false);
    vs.showScalarBar = json["showScalarBar"].toBool(true);
    vs.autoRange     = json["autoRange"].toBool(true);
    vs.rangeMin      = json["rangeMin"].toDouble(0.0);
    vs.rangeMax      = json["rangeMax"].toDouble(100.0);
    vs.isEmmisive    = json["isEmisive"].toBool(false);
    return vs;
}

// =========================================================================
// СХЕМЫ ЧТЕНИЯ
// =========================================================================

QJsonObject ProjectSerializer::serializeReadScheme(const IO::ReadScheme& scheme) {
    return std::visit(
        [](auto&& arg) -> QJsonObject {
            using T = std::decay_t<decltype(arg)>;
            QJsonObject obj;
            if constexpr (std::is_same_v<T, IO::ColumnScheme>) {
                obj = serializeColumnScheme(arg);
                obj.insert("schemeType", "ColumnScheme");
            } else if constexpr (std::is_same_v<T, IO::HDF5ReadScheme>) {
                obj.insert("schemeType", "HDF5ReadScheme");
                obj.insert("path", arg.internalDatasetPath);
                obj.insert("loadAll", arg.loadAll);
                obj.insert("compression", arg.compressionLevel);
            } else if constexpr (std::is_same_v<T, IO::DefaultScheme>) {
                obj.insert("schemeType", "DefaultScheme");
            } else {
                obj.insert("schemeType", "None");
            }
            return obj;
        },
        scheme);
}

IO::ReadScheme ProjectSerializer::deserializeReadScheme(const QJsonObject& json) {
    QString type = json["schemeType"].toString();
    if (type == "ColumnScheme") {
        return deserializeColumnScheme(json);
    }
    if (type == "HDF5ReadScheme") {
        IO::HDF5ReadScheme s;
        s.internalDatasetPath = json["path"].toString();
        s.loadAll             = json["loadAll"].toBool();
        s.compressionLevel    = json["compression"].toInt();
        return s;
    }
    if (type == "DefaultScheme") {
        return IO::DefaultScheme{};
    }
    return std::monostate{};
}

QJsonObject ProjectSerializer::serializeColumnScheme(const IO::ColumnScheme& cs) {
    QJsonObject obj;
    obj.insert("offset", cs.headerOffsetBytes);
    obj.insert("interleaved", cs.isInterleaved);
    obj.insert("delimiter", cs.delimiter);

    QJsonArray cols;
    for (const auto& m : cs.columnsPolicy) {
        QJsonObject mObj;
        mObj.insert("name", m.name);
        mObj.insert("vtkType", m.vtkDataType);
        mObj.insert("role", m.vtkAttributeRole);
        mObj.insert("components", m.numberOfComponents);
        mObj.insert("isCoord", m.isCoordiante);
        cols.append(mObj);
    }
    obj.insert("columns", cols);
    return obj;
}

IO::ColumnScheme ProjectSerializer::deserializeColumnScheme(const QJsonObject& json) {
    IO::ColumnScheme cs;
    cs.headerOffsetBytes = json["offset"].toInt();
    cs.isInterleaved     = json["interleaved"].toBool();
    cs.delimiter         = json["delimiter"].toString();

    QJsonArray cols = json["columns"].toArray();
    for (auto v : cols) {
        QJsonObject               mObj = v.toObject();
        IO::ColumnScheme::Mapping m;
        m.name               = mObj["name"].toString();
        m.vtkDataType        = mObj["vtkType"].toInt();
        m.vtkAttributeRole   = mObj["role"].toInt();
        m.numberOfComponents = mObj["components"].toInt();
        m.isCoordiante       = mObj["isCoord"].toBool();
        cs.columnsPolicy.append(m);
    }
    return cs;
}

} // namespace QSpace::Session