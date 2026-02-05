#pragma once
#include "Common/Enums/CommonEnumsIO.h"
#include "IReader.h"
#include "IWriter.h"
#include <memory>
namespace QSpace::IO
{
class IOFactory
{
public:
  static std::unique_ptr<IReader> createReader(IO::FileFormat format); // строго задаем формат
  static std::unique_ptr<IReader> createReader(const QString &path);   // автоматически определяет формат по расширению
  static std::unique_ptr<IWriter> createWriter(IO::FileFormat format); // строго задаем формат файла при записи
  static std::unique_ptr<IWriter> createWriter(const QString &path);
};
} // namespace QSpace::IO