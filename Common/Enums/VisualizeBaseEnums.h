#pragma once
#include <QList>
#include <QString>
#include <optional>
#include <qcontainerfwd.h>

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

enum class ScalarBarRangeInterpolation
{
  Sigmoid,
  Asinh,
  Linear
};
enum class InterpolationOpacityFunction
{
  Linear,
  Sqrt,
  Sigmoid,
  Asinh,
  Quadro,
  Qube,
  Constant
};
//====================Get List=======================
inline QList<InterpolationOpacityFunction> getAllInterpolationOpacityFunctions()
{
  return {InterpolationOpacityFunction::Constant, InterpolationOpacityFunction::Linear, InterpolationOpacityFunction::Sqrt, InterpolationOpacityFunction::Quadro, InterpolationOpacityFunction::Qube, InterpolationOpacityFunction::Sigmoid, InterpolationOpacityFunction::Asinh};
}

inline QList<ScalarBarRangeInterpolation> getAllScalarBarRangeInterpolationTypes()
{
  return {ScalarBarRangeInterpolation::Asinh, ScalarBarRangeInterpolation::Sigmoid, ScalarBarRangeInterpolation::Linear};
}

//=====================To String=========================
inline QString interpolationOpacityFunctionToString(InterpolationOpacityFunction type)
{
  switch (type)
  {
  case InterpolationOpacityFunction::Linear:
    return "Linear";
  case InterpolationOpacityFunction::Sqrt:
    return "Sqrt";
  case InterpolationOpacityFunction::Quadro:
    return "Quadro";
  case InterpolationOpacityFunction::Qube:
    return "Qube";
  case InterpolationOpacityFunction::Sigmoid:
    return "Sigmoid";
  case InterpolationOpacityFunction::Asinh:
    return "Asinh";
  case InterpolationOpacityFunction::Constant:
    return "Constant";
  default:
    return "Unknown";
  }
}

inline QString scalarBarRangeInterpolationTypeToString(ScalarBarRangeInterpolation type)
{
  switch (type)
  {
  case QSpace::Visualize::ScalarBarRangeInterpolation::Asinh:
    return "Asinh";
  case QSpace::Visualize::ScalarBarRangeInterpolation::Sigmoid:
    return "Sigmoid";
  case QSpace::Visualize::ScalarBarRangeInterpolation::Linear:
    return "Linear";
  default:
    return "Unknown";
  }
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

//=====================From String=========================
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

inline std::optional<ScalarBarRangeInterpolation> scalarBarRangeInterpolationFromString(const QString &s)
{
  if (s == "Asinh")
    return QSpace::Visualize::ScalarBarRangeInterpolation::Asinh;
  if (s == "Sigmoid")
    return QSpace::Visualize::ScalarBarRangeInterpolation::Sigmoid;
  if (s == "Linear")
    return QSpace::Visualize::ScalarBarRangeInterpolation::Linear;
  return std::nullopt;
}

inline std::optional<InterpolationOpacityFunction> interpolationOpacityFunctionFromString(const QString &s)
{
  if (s == "Linear")
    return QSpace::Visualize::InterpolationOpacityFunction::Linear;
  if (s == "Sqrt")
    return QSpace::Visualize::InterpolationOpacityFunction::Sqrt;
  if (s == "Quadro")
    return QSpace::Visualize::InterpolationOpacityFunction::Quadro;
  if (s == "Qube")
    return QSpace::Visualize::InterpolationOpacityFunction::Qube;
  if (s == "Sigmoid")
    return QSpace::Visualize::InterpolationOpacityFunction::Sigmoid;
  if (s == "Asinh")
    return QSpace::Visualize::InterpolationOpacityFunction::Asinh;
  if (s == "Constant")
    return QSpace::Visualize::InterpolationOpacityFunction::Constant;
  return std::nullopt;
}
} // namespace QSpace::Visualize