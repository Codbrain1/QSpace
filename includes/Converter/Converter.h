#pragma once
#include <array>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "AgregateStructures.h"
#include "ConstantsParametrs.h"
#include "DataStorage.h"
class Converter
{
   protected:
    using ProjectionRefs =
        std::tuple<std::reference_wrapper<std::vector<double>>, std::reference_wrapper<std::vector<double>>,
                   std::reference_wrapper<std::vector<double>>>;

    std::vector<std::string_view> Z_grd_list;  // колонки которые следует записывать в файлы
    std::pair<std::string_view, std::string_view> XY;
    size_t Nfiles_into_clomun;
    std::filesystem::path output_directory;
    std::vector<FILE*> ofiles;                        // набор выходных файлов (запись по принципу скользящего окна)
    std::vector<std::filesystem::path> ofiles_names;  // имена выходных файлов
    DataStorage& data;                                // хранилище входных данных
    std::vector<std::vector<double>> Z;               // буфер для накопления выходных данных и последующей записи
    size_t obuff_size;                                // число файлов записываемых за раз на диск(1вх ф. --> Z_n вых ф.)
    size_t ofile_buff_size;                           // буфер записи для одного файла
    lim<double> limits_x;                             // ограничение области проецирования по x
    lim<double> limits_y;                             // ограничение области проецирования по y
    lim<double> bound_x;                              // ограничение области выбора частиц по x
    lim<double> bound_y;                              // ограничение области выбора частиц по y
    lim<double> bound_z;                              // ограничение области выбора частиц по z (толщина слоя)
    count_cell Nb_XY;                                 // число ячеек сетки по x и по y в grd файле
    lim<double> fx_lim;                               // нужно для перевода в размерные величины
    lim<double> fy_lim;                               // нужно для перевода в размерные величины
    std::vector<std::vector<lim<double>>> limits_f;   // предельные значения выходной функции на сетке
    SistemCoordinate_Rectangle SC_rectangle;
    SistemCoordinate_Area SC_Area;
    double hb;                               // размер ячейки
    double _hb2;                             // 1/hb^2
    double l_v;                              // l_v = 65.76 * sqrt(Km / Kr);          --> km/s
    double l_r;                              // l_r = 10 * Kr;                        --> kpc
    double l_s;                              // l_s = Km / (Kr * Kr) * 100.0;         --> Msun/pc^2
    double l_rho;                            // l_rho = Km / (Kr * Kr * Kr) * 0.01;   --> Msun/pc^3
    double l_t;                              // l_t = l_r / l_v * 1000.0 * 0.9784;    --> Myr
    double _dV;                              //_hb2 / (zmax - zmin);
    static constexpr double eps_lg = 1e-15;  // логарифмирование входных значений
    size_t all_files_cursor;
    Converter(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy, std::filesystem::path _output_path);
    Converter() = delete;
    void parallel_save_txt(size_t current_cursor, size_t col);
    void consistent_save_txt(size_t current_cursor, size_t col);
    void parallel_save_bin(size_t current_cursor, size_t col);
    void consistent_save_bin(size_t current_cursor, size_t col);
    ProjectionRefs get_projection();

   public:
    void save_grd_txt();  // сохранения в текстовый файл
    void save_grd_bin();  // сохранения в бинарный файл
    void set_boundary(lim<double> x, lim<double> y, lim<double> z, double alpha = 0, double beta = 0, double phi = 0);
    void set_limits(lim<double> x, lim<double> y, double teta = 0, double psi = 0);
    void set_obuff_size(const size_t size);
    void set_ofile_buff_size(const size_t size);
    void setup_output_data(const std::vector<std::string>& Z_grd_list_columns,
                           const std::pair<std::string, std::string>& _XY);
    std::string extract_upper_axis(const std::string& input);  //"x (текст)" → "X", "y (любой текст)" → "Y"

   protected:
    void create_output_directory(std::string first_part, std::string second_part, std::string thrid_part, size_t files_size);
    // вычисление проекций скоростей
    template <class Predicate>
    void calculate_V_projection(std::vector<double>& projection1, std::vector<double>& projection2,
                                std::vector<double>& projection3, std::vector<double>& v_projection,
                                const Predicate& condition);
    // вычисление поверхностной плотности
    template <class Predicate>
    void calculate_Sigma(std::vector<double>& projection1, std::vector<double>& projection2,
                         std::vector<double>& projection3, const Predicate& condition);
    template <class Predicate>
    std::array<double, 3> calculate_Center_of_Mass(int i_file, const Predicate& condition);  // центр масс

    template <class Predicate>
    std::vector<std::array<double, 3>> calculate_P(const Predicate& condition);  // полный импульс

    template <class Predicate>
    std::vector<std::array<double, 3>> calculate_L(const Predicate& condition);  // момент импульса

    template <class Predicate>
    void calculate_Vr(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3,
                      const Predicate& condition);
    template <class Predicate>
    void calculate_Vfi(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3,
                       const Predicate& condition);
};

#include "Converter.tpp"