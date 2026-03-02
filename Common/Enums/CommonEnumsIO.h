#pragma once
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
} // namespace QSpace::IO