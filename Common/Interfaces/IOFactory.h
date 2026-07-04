#pragma once
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "IReader.h"
#include "IWriter.h"
#include <memory>
#include <qcontainerfwd.h>
namespace QSpace::IO
{
std::unique_ptr<IReader> createReader(FileFormat format); // строго задаем формат
std::unique_ptr<IWriter> createWriter(FileFormat format); // строго задаем формат файла при записи
namespace Utils
{
FileFormat getFormat(const QString &path);
QSpace::Visualize::EntityType getEntityType(const QString &name);
}; // namespace Utils
} // namespace QSpace::IO