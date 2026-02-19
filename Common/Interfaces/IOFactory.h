#pragma once
#include "Common/Enums/CommonEnumsIO.h"
#include "Enums/RenderEnums.h"
#include "IReader.h"
#include "IWriter.h"
#include <memory>
#include <qcontainerfwd.h>
namespace QSpace::IO
{
class IOFactory
{
public:
  static std::unique_ptr<IReader> createReader(IO::FileFormat format); // строго задаем формат
  static std::unique_ptr<IWriter> createWriter(IO::FileFormat format); // строго задаем формат файла при записи
};
class Utils
{
public:
  static FileFormat getFormat(const QString &path);
  static Visualize::EntityType getEntityType(const QString &name);
}; // namespace Utils
} // namespace QSpace::IO