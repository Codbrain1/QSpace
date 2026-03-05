#pragma once
#include "ProjectSerializer.h"
#include <qstringview.h>
namespace QSpace::Session {
class SessionStorage {
  public:
    static bool                      save(const QString& filePath, const QByteArray& data);
    static std::optional<QByteArray> load(const QString& filePath);
};
} // namespace QSpace::Session