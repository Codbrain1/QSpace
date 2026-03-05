#include "Common/Interfaces/IOFactory.h"
#include "Common/Enums/IOEnums.h"
#include "IO/BIN/BINReader.h"
#include "Interfaces/IReader.h"
#include <memory>
#include <qcontainerfwd.h>
#include <qnamespace.h>
namespace QSpace::IO {
std::unique_ptr<IReader> IOFactory::createReader(IO::FileFormat format) {
    if (format == FileFormat::BIN) {
        return std::make_unique<BINReader>();
    } else // TODO:: добавить другие форматы
        return nullptr;
}
FileFormat Utils::getFormat(const QString& path) {
    if (path.endsWith(".bin", Qt::CaseInsensitive)) {
        return FileFormat::BIN;
    } else if (path.endsWith(".grd", Qt::CaseInsensitive)) {
        return FileFormat::GRD;
    } else if (path.endsWith(".hdf5", Qt::CaseInsensitive)) {
        return FileFormat::HDF5;
    } else if (path.endsWith(".txt", Qt::CaseInsensitive)) {
        return FileFormat::TXT;
    }
    return FileFormat::Unknown;
}
Visualize::EntityType Utils::getEntityType(const QString& name) {
    QString n = name.toUpper();
    if (n.contains("GAS") || n.contains("G"))
        return Visualize::EntityType::Gas;
    if (n.contains("STAR") || n.contains("S"))
        return Visualize::EntityType::Stars;
    if (n.contains("DM") || n.contains("DARK"))
        return Visualize::EntityType::DarkMatter;
    return Visualize::EntityType::Unknown;
}
} // namespace QSpace::IO