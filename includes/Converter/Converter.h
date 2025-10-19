#pragma once
#include <cstddef>
#include <filesystem>
#include <string_view>
#include <vector>

#include "AgregateStructures.h"
#include "ConstantsParametrs.h"
#include "DataStorage.h"
class Converter
{
   protected:
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
    lim<double> limits_z;                             // ограничение области проецирования по z (толщина слоя)
    count_cell Nb_XY;                                 // число ячеек сетки по x и по y в grd файле
    lim<double> fx_lim;                               // нужно для перевода в размерные величины
    lim<double> fy_lim;                               // нужно для перевода в размерные величины
    std::vector<std::vector<lim<double>>> limits_f;   // предельные значения выходной функции на сетке
    double hb;                                        // размер ячейки
    double _hb2;                                      // 1/hb^2
    double l_v;                                       // l_v = 65.76 * sqrt(Km / Kr);          --> km/s
    double l_r;                                       // l_r = 10 * Kr;                        --> kpc
    double l_s;                                       // l_s = Km / (Kr * Kr) * 100.0;         --> Msun/pc^2
    double l_rho;                                     // l_rho = Km / (Kr * Kr * Kr) * 0.01;   --> Msun/pc^3
    double l_t;                                       // l_t = l_r / l_v * 1000.0 * 0.9784;    --> Myr
    double _dV;                                       //_hb2 / (zmax - zmin);
    static constexpr double eps_lg = 1e-15;           // логарифмирование входных значений

    Converter(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy, std::filesystem::path _output_path);
    Converter() = delete;
    void parallel_save_txt(size_t current_cursor, size_t col);
    void consistent_save_txt(size_t current_cursor, size_t col);
    void parallel_save_bin(size_t current_cursor, size_t col);
    void consistent_save_bin(size_t current_cursor, size_t col);

   public:
    void save_grd_txt();  // сохранения в текстовый файл
    void save_grd_bin();  // сохранения в бинарный файл
    void set_limits(lim<double> x, lim<double> y, lim<double> z);
    void set_obuff_size(const size_t size);
    void set_ofile_buff_size(const size_t size);
    void setup_output_data(const std::vector<std::string>& Z_grd_list_columns,
                           const std::pair<std::string_view, std::string_view>& _XY);
};