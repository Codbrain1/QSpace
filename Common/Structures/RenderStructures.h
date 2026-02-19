#pragma once
#include "Common/Enums/RenderEnums.h"
#include <QVector>
#include <qlist.h>
namespace QSpace::Visualize {
struct ColorPoint {
    double x;       // Позиция на шкале (0.0 - 1.0)
    double r, g, b; // Цвет (0.0 - 1.0)
};
class ColorMapRegistry {
  public:
    static QVector<ColorPoint> getPresetPoints(ColorMapType type) {
        switch (type) {
            case ColorMapType::Viridis:
                return {{0.00, 0.267, 0.004, 0.329},
                        {0.25, 0.230, 0.322, 0.545},
                        {0.50, 0.127, 0.566, 0.550},
                        {0.75, 0.369, 0.788, 0.382},
                        {1.00, 0.993, 0.906, 0.143}};
            case ColorMapType::Inferno:
                return {{0.00, 0.001, 0.000, 0.004},
                        {0.25, 0.330, 0.007, 0.370},
                        {0.50, 0.730, 0.210, 0.230},
                        {0.75, 0.980, 0.640, 0.160},
                        {1.00, 0.980, 0.990, 0.690}};
            case ColorMapType::Plasma:
                return {{0.00, 0.050, 0.020, 0.520},
                        {0.25, 0.410, 0.040, 0.650},
                        {0.50, 0.740, 0.200, 0.540},
                        {0.75, 0.950, 0.510, 0.260},
                        {1.00, 0.940, 0.940, 0.130}};
            case ColorMapType::Magma:
                return {{0.00, 0.000, 0.000, 0.010},
                        {0.25, 0.140, 0.040, 0.310},
                        {0.50, 0.440, 0.080, 0.490},
                        {0.75, 0.800, 0.220, 0.380},
                        {1.00, 0.980, 0.920, 0.700}};
            case ColorMapType::CoolToWarm:
                return {{0.00, 0.230, 0.290, 0.750}, {0.50, 0.860, 0.860, 0.860}, {1.00, 0.700, 0.010, 0.140}};
            case ColorMapType::Rainbow:
                return {
                    {0.00, 0.000, 0.000, 1.000}, // Синий
                    {0.25, 0.000, 1.000, 1.000}, // Циан
                    {0.50, 0.000, 1.000, 0.000}, // Зеленый
                    {0.75, 1.000, 1.000, 0.000}, // Желтый
                    {1.00, 1.000, 0.000, 0.000}  // Красный
                };
            case ColorMapType::Grayscale:
                return {
                    {0.00, 0.000, 0.000, 0.000}, // Черный
                    {1.00, 1.000, 1.000, 1.000}  // Белый
                };
            default:
                return getPresetPoints(ColorMapType::Viridis);
        }
    }

    // Хелпер для UI (получить список имен)
    static QString toString(ColorMapType type) {
        switch (type) {
            case ColorMapType::Viridis:
                return "Viridis";
            case ColorMapType::Inferno:
                return "Inferno";
            case ColorMapType::Plasma:
                return "Plasma";
            case ColorMapType::Magma:
                return "Magma";
            case ColorMapType::CoolToWarm:
                return "CoolToWarm";
            case ColorMapType::Rainbow:
                return "Rainbow";
            case ColorMapType::Grayscale:
                return "Grayscale";
            default:
                return "Unknown";
        }
    }
    static QList<ColorMapType> getAllTypes() {
        return {ColorMapType::Viridis,
                ColorMapType::Inferno,
                ColorMapType::Plasma,
                ColorMapType::Magma,
                ColorMapType::CoolToWarm,
                ColorMapType::Rainbow,
                ColorMapType::Grayscale};
    }
};
} // namespace QSpace::Visualize