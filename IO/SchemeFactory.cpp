#include "SchemeFactory.h"

namespace QSpace::IO::SchemeFactory {
ReadScheme createScheme_v2_3(Visualize::EntityType type, FileFormat format) {
    if (format == FileFormat::BIN || format == FileFormat::TXT) {
        ColumnScheme scheme;
        if (type == QSpace::Visualize::EntityType::DarkMatter ||
            type == QSpace::Visualize::EntityType::Stars) {
            scheme.columnsPolicy.append({"Position", VTK_DOUBLE, -1, 3, true});
            scheme.columnsPolicy.append(
                {"Velocity", VTK_DOUBLE, vtkDataSetAttributes::VECTORS, 3, false});
            scheme.columnsPolicy.append(
                {"Mass", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
        } else if (type == QSpace::Visualize::EntityType::Gas) {
            scheme.columnsPolicy.append({"Position", VTK_DOUBLE, -1, 3, true});
            scheme.columnsPolicy.append(
                {"Density", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
            scheme.columnsPolicy.append(
                {"Velocity", VTK_DOUBLE, vtkDataSetAttributes::VECTORS, 3, false});
            scheme.columnsPolicy.append(
                {"Energy", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
            scheme.columnsPolicy.append(
                {"Mass", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
            scheme.columnsPolicy.append(
                {"ind_SPH", VTK_INT, vtkDataSetAttributes::SCALARS, 1, false});
            scheme.columnsPolicy.append(
                {"t_MCYS", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
        }
        scheme.isInterleaved = true;
        return scheme;
    }
    // MINOR::добавить обработку других форматов hdf5 и тд
    return DefaultScheme{};
}

ReadScheme createScheme_v2(Visualize::EntityType type, FileFormat format) {
    if (format == FileFormat::BIN || format == FileFormat::TXT) {
        ColumnScheme scheme;
        if (type == QSpace::Visualize::EntityType::DarkMatter ||
            type == QSpace::Visualize::EntityType::Stars) {
            scheme.columnsPolicy.append({"Position", VTK_DOUBLE, -1, 3, true});
            scheme.columnsPolicy.append(
                {"Velocity", VTK_DOUBLE, vtkDataSetAttributes::VECTORS, 3, false});
            scheme.columnsPolicy.append(
                {"Mass", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
        } else if (type == QSpace::Visualize::EntityType::Gas) {
            scheme.columnsPolicy.append({"Position", VTK_DOUBLE, -1, 3, true});
            scheme.columnsPolicy.append(
                {"Density", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
            scheme.columnsPolicy.append(
                {"Velocity", VTK_DOUBLE, vtkDataSetAttributes::VECTORS, 3, false});
            scheme.columnsPolicy.append(
                {"Energy", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
            scheme.columnsPolicy.append(
                {"Mass", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
            scheme.columnsPolicy.append(
                {"ind_SPH", VTK_INT, vtkDataSetAttributes::SCALARS, 1, false});
            // scheme.columnsPolicy.append({"t_MCYS", VTK_DOUBLE, vtkDataSetAttributes::SCALARS,
            // 1, false});
        }
        // MINOR:: //добавить обработку MIXED
        scheme.isInterleaved = true;
        return scheme;
    }
    // MINOR::добавить обработку других форматов hdf5 и тд
    return DefaultScheme{};
}
} // namespace QSpace::IO::SchemeFactory