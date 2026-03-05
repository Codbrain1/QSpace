#pragma once
#include "Common/Enums/IOEnums.h"
#include "Common/Enums/RenderEnums.h"
#include "Common/Structures/IOStructures.h"
#include <QList>
#include <QMap>
#include <QString>
#include <qcontainerfwd.h>
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkSmartPointer.h>
#include <vtkType.h>


namespace QSpace::IO {
struct ColumnScheme {
    struct Mapping {
        QString name;        // название столбца (например density)
        int     vtkDataType; // тип данных из vtk
        int     vtkAttributeRole;
        int     numberOfComponents;
        bool    isCoordiante; // 0 для X, 1 для Y, 2 для Z, и -1 если не координата
    };
    QList<Mapping> columnsPolicy;
    int            headerOffsetBytes = 0;     // смещение относительно заголовка
    bool           isInterleaved     = false; // true если X1,Y1,Z1, X2,Y2,Z2.
    QString        delimiter         = " ";   // для TXT файла
};
struct HDF5ReadScheme {
    QString internalDatasetPath;     // внутренний путь к данным например "/Particles/Position"
    bool    loadAll          = true; // загружать ли весь файл
    int     compressionLevel = 0;    // коофициент сжатия 0-9
};
struct HDF5WriteScheme {
    QString rootGroupPath; // путь к группе данных Например: "/TimeStep_001"
    // Если список пуст — пишем ВСЕ массивы из vtkDataSet.
    // Если заполнен — пишем только указанные, меняя имя vtk -> hdf5
    // Ключ: Имя массива в VTK (например "Density"), Значение: Имя в HDF5 ("rho")
    QMap<QString, QString> arrayMapping;
};
struct DefaultScheme {}; // для VTK и GRD нет определенной схемы так как имеют строгий неизменный формат
using ReadScheme  = std::variant<std::monostate, ColumnScheme, HDF5ReadScheme, DefaultScheme>;
using WriteScheme = std::variant<std::monostate, ColumnScheme, HDF5WriteScheme, DefaultScheme>;

// ----- возвращаемые значения IReader -----
//
struct ReadResult { // результат чтения одного файла
    vtkSmartPointer<vtkDataSet> data = nullptr;
    QString                     path;
    QString                     errMessage;
    IO::FileFormat              format;
    IO::ReadScheme              scheme;
    ReadStatus                  status = ReadStatus::UnknownError;
    bool                        isSuccess() const {
        return status == ReadStatus::Succes && data != nullptr;
    }
};
struct BatchResult { // результат чтения пакета файлов
    vtkSmartPointer<vtkMultiBlockDataSet> data         = nullptr;
    int                                   succes_count = 0;
    int                                   fail_count   = 0;
    QStringList                           errMessages;
    bool                                  hasErrors() {
        return fail_count > 0;
    }
};
struct BatchTask { // одна задача для пакетного чтения
    QString    path;
    ReadScheme scheme;
};
class SchemeFactory {
  public:
    static ReadScheme createDefaultSheme(Visualize::EntityType type, FileFormat format) {
        if (format == FileFormat::BIN || format == FileFormat::TXT) {
            ColumnScheme scheme;
            if (type == QSpace::Visualize::EntityType::DarkMatter || type == QSpace::Visualize::EntityType::Stars) {
                scheme.columnsPolicy.append({"Position", VTK_DOUBLE, -1, 3, true});
                scheme.columnsPolicy.append({"Velocity", VTK_DOUBLE, vtkDataSetAttributes::VECTORS, 3, false});
                scheme.columnsPolicy.append({"Mass", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
            } else if (type == QSpace::Visualize::EntityType::Gas) {
                scheme.columnsPolicy.append({"Position", VTK_DOUBLE, -1, 3, true});
                scheme.columnsPolicy.append({"Density", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
                scheme.columnsPolicy.append({"Velocity", VTK_DOUBLE, vtkDataSetAttributes::VECTORS, 3, false});
                scheme.columnsPolicy.append({"Energy", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
                scheme.columnsPolicy.append({"Mass", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
                scheme.columnsPolicy.append({"ind_SPH", VTK_INT, vtkDataSetAttributes::SCALARS, 1, false});
                scheme.columnsPolicy.append({"t_MCYS", VTK_DOUBLE, vtkDataSetAttributes::SCALARS, 1, false});
            }
            // TODO:: //добавить обработку MIXED
            scheme.isInterleaved = true;
            return scheme;
        }
        // TODO::добавить обработку других форматов hdf5 и тд
        return DefaultScheme{};
    }
};
} // namespace QSpace::IO