#include "Common/Interfaces/IOFactory.h"
#include "Enums/CommonEnumsIO.h"
#include "IO/BIN/BINReader.h"
#include "Interfaces/IReader.h"
#include <memory>
#include <qcontainerfwd.h>
#include <qnamespace.h>
namespace QSpace::IO {
std::unique_ptr<IReader> IOFactory::createReader(IO::FileFormat format) {
    if (format == FileFormat::BIN) {
        return std::make_unique<BINReader>();
    } else
        return nullptr;
}
std::unique_ptr<IReader> IOFactory::createReader(const QString& path) {
    if (path.endsWith(".bin", Qt::CaseInsensitive)) {
        return std::make_unique<BINReader>();
    } else
        return nullptr;
}
} // namespace QSpace::IO