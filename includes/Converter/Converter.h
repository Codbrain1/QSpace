#pragma once
#include <filesystem>
#include <string_view>
#include <vector>

#include "AgregateStructures.h"
#include "DataStorage.h"
class Converter
{
   protected:
    std::vector<std::string_view> XYZ_grd_list;       // колонки которые следует записывать в файлы
    std::vector<FILE> ofiles;                         // набор выходных файлов (запись по принципу скользящего окна)
    std::vector<std::filesystem::path> ofiles_names;  // имена выходных файлов
    DataStorage& data;                                // хранилище входных данных
    std::vector<std::vector<double>> Z;               // буфер для накопления выходных данных и последующей записи
    int obuff_size;                                   // число файлов записываемых за раз на диск
    int ofile_buff_size;                              // буфер записи для одного файла
    lim<double> limits_x;                             // ограничение области проецирования по x
    lim<double> limits_y;                             // ограничение области проецирования по y
    lim<double> limits_z;                             // ограничение области проецирования по z (толщина слоя)
    lim<double> limits_f;                             // предельные значения выходной функции на сетке
    count_cell Nb_XY;                                 // число ячеек сетки по x и по y в grd файле
    lim<double> fx_lim;                               // нужно для перевода в размерные величины
    lim<double> fy_lim;                               // нужно для перевода в размерные величины
    double hb;                                        // размер ячейки
    double hb2;                                       // 1/hb^2
    double l_v;                                       // l_v = 65.76 * sqrt(Km / Kr);          --> km/s
    double l_r;                                       // l_r = 10 * Kr;                        --> kpc
    double l_s;                                       // l_s = Km / (Kr * Kr) * 100.0;         --> Msun/pc^2
    double l_rho;                                     // l_rho = Km / (Kr * Kr * Kr) * 0.01;   --> Msun/pc^3
    double l_t;                                       // l_t = l_r / l_v * 1000.0 * 0.9784;    --> Myr
    static constexpr double eps_lg = 1e-15;           // логарифмирование ваходных значений

   public:
    void convert();       // функция конвертации
    void save_grd_txt();  // сохранения в текстовый файл
    void save_grd_bin();  // сохранения в бинарный файл
    Converter();
    void set_limits(lim<double> x, lim<double> y, lim<double> z);
    void st_obuff_size(const int size);
    void st_ofile_buff_size(const int size);
};