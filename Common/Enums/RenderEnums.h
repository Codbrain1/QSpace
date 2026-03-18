#pragma once
#include <QList>
#include <QString>
#include <optional>

namespace QSpace::Visualize
{
enum class EntityType
{
  Unknown,
  Gas,
  Stars,
  DarkMatter,
  Mixed
};
enum class RenderMode
{
  Points,
  GausianSplat,
  Volume
};
// enum class ColorMapType
// {
//   Viridis,
//   Inferno,
//   Plasma,
//   Magma,
//   CoolToWarm,
//   Rainbow,
//   Grayscale
// };
enum class CameraViewType
{
  XY_Top,   // Сверху
  XZ_Front, // Спереди
  YZ_Right, // Справа
  Iso       // Изометрия
  // TODO:: добавить стандратные позиции
};
inline QList<QSpace::Visualize::RenderMode> getAllRenderModes()
{
  return {RenderMode::GausianSplat, RenderMode::Points, RenderMode::Volume};
}
inline QString entitytypeToString(EntityType type)
{
  switch (type)
  {
  case QSpace::Visualize::EntityType::DarkMatter:
    return "DarkMatter";
  case QSpace::Visualize::EntityType::Gas:
    return "Gas";
  case QSpace::Visualize::EntityType::Stars:
    return "Stars";
  case QSpace::Visualize::EntityType::Mixed:
    return "Mixed";
  default:
    return "Unknown";
  }
}
inline EntityType entitytypeFromString(const QString &s)
{
  if (s == "DarkMatter")
    return EntityType::DarkMatter;
  if (s == "Gas")
    return EntityType::Gas;
  if (s == "Stars")
    return EntityType::Stars;
  if (s == "Mixed")
    return EntityType::Mixed;
  return EntityType::Unknown;
}
inline QString rendermodeToString(RenderMode mode)
{
  switch (mode)
  {
  case QSpace::Visualize::RenderMode::GausianSplat:
    return "GausianSplat";
  case QSpace::Visualize::RenderMode::Points:
    return "Points";
  case QSpace::Visualize::RenderMode::Volume:
    return "Volume";
  default:
    return "Unknown";
  }
}
inline std::optional<RenderMode> rendermodeFromString(const QString &s)
{
  if (s == "GausianSplat")
    return QSpace::Visualize::RenderMode::GausianSplat;
  if (s == "Points")
    return QSpace::Visualize::RenderMode::Points;
  if (s == "Volume")
    return QSpace::Visualize::RenderMode::Volume;
  return std::nullopt;
}
} // namespace QSpace::Visualize