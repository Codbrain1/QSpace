#pragma once
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
enum class ColorMapType
{
  Viridis,
  Inferno,
  Plasma,
  Magma,
  CoolToWarm,
  Rainbow,
  Grayscale
};
enum class CameraViewType
{
  XY_Top,   // Сверху
  XZ_Front, // Спереди
  YZ_Right, // Справа
  Iso       // Изометрия
  // TODO:: добавить стандратные позиции
};
} // namespace QSpace::Visualize