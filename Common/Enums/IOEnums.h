#pragma once
#include <QString>
#include <qcontainerfwd.h>
namespace QSpace::IO
{
enum class ReadStatus
{
  Succes,
  FileNotFound,
  InvalidFormat,
  InvalidFileStructure,
  UnknownError
};
enum class FilePolicy
{
  Auto,         // автоматическое определение способа чтения исходя из объемов данных
  ForceMapped,  // используется Qmap для проэцирования файлов в виртуальную память
  ForceStandart // обычное чтение через стандартные QFile
};
enum class FileFormat
{
  BIN,  // специфичный бинарный формат
  TXT,  // специфичный текстовый формат
  GRD,  // формат Surfer представляет собой реглярную стеку с значениями
  HDF5, // формат для больших данных, включает архивирование
  Unknown
};
enum class ImportRole
{
  ProjectData, // Обычные данные пользователя
  Internal     // Временные данные (например, кадры видео)
};
inline QString fileformatToString(FileFormat format)
{
  switch (format)
  {
  case QSpace::IO::FileFormat::BIN:
    return "BIN";
  case QSpace::IO::FileFormat::GRD:
    return "GRD";
  case QSpace::IO::FileFormat::HDF5:
    return "HDF5";
  case QSpace::IO::FileFormat::TXT:
    return "TXT";
  default:
    return "Unknown";
  }
}
inline FileFormat fileformatFromString(QString s)
{
  if (s == "BIN")
    return QSpace::IO::FileFormat::BIN;
  if (s == "GRD")
    return QSpace::IO::FileFormat::GRD;
  if (s == "HDF5")
    return QSpace::IO::FileFormat::HDF5;
  if (s == "TXT")
    return QSpace::IO::FileFormat::TXT;
  return FileFormat::Unknown;
}
} // namespace QSpace::IO