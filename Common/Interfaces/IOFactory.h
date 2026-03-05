#pragma once
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "IReader.h"
#include "IWriter.h"
#include <memory>
#include <qcontainerfwd.h>
namespace QSpace::IO
{
class IOFactory
{
public:
  static std::unique_ptr<IReader> createReader(QSpace::IO::FileFormat format); // строго задаем формат
  static std::unique_ptr<IWriter> createWriter(QSpace::IO::FileFormat format); // строго задаем формат файла при записи
};
class Utils
{
public:
  static QSpace::IO::FileFormat getFormat(const QString &path);
  static QSpace::Visualize::EntityType getEntityType(const QString &name);
}; // namespace Utils
} // namespace QSpace::IO