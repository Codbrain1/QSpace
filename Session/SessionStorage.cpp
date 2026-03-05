#include "SessionStorage.h"
#include <QFile>
#include <optional>
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <qstringview.h>
namespace QSpace::Session {
bool SessionStorage::save(const QString& filePath, const QByteArray& data) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    return file.write(data) != -1;
}
std::optional<QByteArray> SessionStorage::load(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }
    return file.readAll();
}
} // namespace QSpace::Session