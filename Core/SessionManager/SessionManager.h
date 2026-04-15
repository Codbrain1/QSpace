#pragma once
#include "Core/ObjectRegistry/ObjectRegistry.h"
#include "Session/SessionStorage.h"
#include "Structures/SessionStructures.h"
#include <QObject>
#include <memory>
#include <optional>

namespace QSpace::Core {
class SessionManager : public QObject { // Исправлена опечатка в имени
    Q_OBJECT
  public:
    explicit SessionManager(QSpace::Core::ObjectRegistry* registry, QObject* parent = nullptr);

    bool                                         saveProject(const QSpace::Session::CurrentSession& curSession);
    std::optional<QSpace::Session::ProjectState> loadProject(const QString& filePath);
    bool                                         savePalette(const Visualize::ColorMap& map, const QString& filePath);
    std::optional<Visualize::ColorMap>           loadPalette(const QString& filePath);

  signals:
    void projectLoaded(const QSpace::Session::ProjectState& state);
    void errorOccurred(const QString& msg);

  private:
    QSpace::Core::ObjectRegistry*                    m_registry;
    std::unique_ptr<QSpace::Session::SessionStorage> m_storage_session;
};
} // namespace QSpace::Core