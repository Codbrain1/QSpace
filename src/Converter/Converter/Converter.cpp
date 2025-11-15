#include "Converter/Converter.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <utility>
#include <vector>
Converter::Converter(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy, std::filesystem::path _output_path)
    : Nfiles_into_clomun(_data.get_ibuff_size()),
      output_directory(_output_path),
      data(_data),
      obuff_size(_data.get_ibuff_size()),
      ofile_buff_size(16384),
      limits_x(0, 0),
      limits_y(0, 0),
      bound_x(0, 0),
      bound_y(0, 0),
      bound_z(0, 0),
      Nb_XY(_Nbxy),
      fx_lim(0, 0),
      fy_lim(0, 0),
      hb(c.hb),
      all_files_cursor(0)
{
    _hb2 = 1.0 / (hb * hb);
    l_v = 65.76 * std::sqrt(c.Km / c.Kr);        //--> km/s
    l_r = 10 * c.Kr;                             //--> kpc
    l_s = c.Km / (c.Kr * c.Kr) * 100.0;          //--> Msun/pc^2
    l_rho = c.Km / (c.Kr * c.Kr * c.Kr) * 0.01;  //--> Msun/pc^3
    l_t = l_r / l_v * 1000.0 * 0.9784;           //--> Myr
    _dV = 0;
}
void Converter::set_limits(lim<double> x, lim<double> y, double teta, double psi)
{
    double wx = 0.5 * (x.max - x.min);
    double wy = 0.5 * (y.max - y.min);
    limits_x = {wx, -wx};
    limits_y = {wy, -wy};
    fx_lim.min = x.min * l_r;
    fy_lim.min = y.min * l_r;
    fx_lim.max = x.max * l_r;
    fy_lim.max = y.max * l_r;
    SC_Area.setup_SC(x, y, teta, psi);
}
void Converter::set_boundary(lim<double> x, lim<double> y, lim<double> z, double alpha, double beta, double phi)
{
    double wx = 0.5 * (x.max - x.min);
    double wy = 0.5 * (y.max - y.min);
    double wz = 0.5 * (z.max - z.min);
    bound_x = {wx, -wx};
    bound_y = {wy, -wy};
    bound_z = {wz, -wz};
    _dV = _hb2 / (z.max - z.min);
    SC_rectangle.setup_SC(x, y, z, alpha, beta, phi);
}
void Converter::set_obuff_size(size_t size)
{
    if (size > 0) {
        obuff_size = size;
    } else {
        throw std::invalid_argument("invalide set obufsize param");
    }
}
void Converter::set_ofile_buff_size(size_t size)
{
    if (size > 0) {
        ofile_buff_size = size;
    } else {
        throw std::invalid_argument("invalide set ofile_buff_size param");
    }
}
void Converter::setup_output_data(const std::vector<std::string>& Z_grd_list_columns,
                                  const std::pair<std::string, std::string>& _XY)
{
    for (const auto& i : Z_grd_list_columns) {
        if (std::find(ParametrsList::Z_outParams.begin(), ParametrsList::Z_outParams.end(), i) !=
                ParametrsList::Z_outParams.end() ||
            i == ParametrsList::Z_outParamT || i == ParametrsList::Z_outParamLgT)
            Z_grd_list.push_back(i);
        else {
            throw std::invalid_argument("неразрешенный тип выходных данных в списке");
        }
    }
    XY = _XY;
}
void Converter::save_grd_txt()
{
    if (obuff_size > ofiles_names.size()) {
        obuff_size = ofiles_names.size();  // если число записываемых за раз файлов меньше чем общее число файлов которое
                                           // требуется записать (учитывает разные столбцы)
    }
    ofiles.resize(obuff_size);
    Nfiles_into_clomun = data.get_ibuff_size();
    for (size_t col = 0; col < Z_grd_list.size(); ++col) {  // запись данных в различные папки
        size_t current_cursor = 0;
        while (current_cursor < Nfiles_into_clomun) {  // шаг по буферу файлов записываемых за раз
            // если файлов осталось меньше чем размер буфера
            if (Nfiles_into_clomun - current_cursor < obuff_size) {
                obuff_size = Nfiles_into_clomun - current_cursor;
            }

            if (obuff_size >= 8) {  // паралельное выполнение если файлов больше или равно 8
                parallel_save_txt(current_cursor, col);
            } else {  // последовательное выполнение если файлов меньше 8
                consistent_save_txt(current_cursor, col);
            }
            current_cursor += obuff_size;
            all_files_cursor += obuff_size;
        }
    }
    Z.clear();
    ofiles.clear();
    ofiles_names.clear();
    limits_f.clear();
}
void Converter::save_grd_bin()
{
    if (obuff_size > ofiles_names.size()) {
        obuff_size = ofiles_names.size();  // если число записываемых за раз файлов меньше чем общее число файлов которое
                                           // требуется записать (учитывает разные столбцы)
    }
    ofiles.resize(obuff_size);
    Nfiles_into_clomun = data.get_ibuff_size();
    for (size_t col = 0; col < Z_grd_list.size(); ++col) {  // запись данных в различные папки
        size_t current_cursor = 0;
        while (current_cursor < Nfiles_into_clomun) {  // шаг по буферу файлов записываемых за раз
            // если файлов осталось меньше чем размер буфера
            if (Nfiles_into_clomun - current_cursor < obuff_size) {
                obuff_size = Nfiles_into_clomun - current_cursor;
            }

            if (obuff_size >= 8) {  // паралельное выполнение если файлов больше или равно 8
                parallel_save_bin(current_cursor, col);
            } else {  // последовательное выполнение если файлов меньше 8
                consistent_save_bin(current_cursor, col);
            }
            current_cursor += obuff_size;
            all_files_cursor += obuff_size;
        }
    }
    Z.clear();
    ofiles.clear();
    ofiles_names.clear();
    limits_f.clear();
}
void Converter::parallel_save_bin(size_t current_cursor, size_t col)
{
#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < obuff_size; ++i) {  // шаг по файлам
        size_t cursor_i = current_cursor + i;
        // открытие и настройка файла
#ifdef _WIN32
        ofiles[cursor_i] = _wfopen(ofiles_names[all_files_cursor + i].wstring().c_str(), L"wb");
#else
        ofiles[cursor_i] = fopen(ofiles_names[all_files_cursor + i].c_str(), "wb");
#endif
        if (!ofiles[cursor_i]) {
            // TODO: добавить логи
        } else {
            std::vector<char> buffer(ofile_buff_size);
            setvbuf(ofiles[cursor_i], buffer.data(), _IOFBF, buffer.size());

            //------------------------------запись данных в файл---------------------------
            fwrite("DSBB", sizeof(char), 4, ofiles[cursor_i]);
            fwrite(&Nb_XY.Nx, sizeof(short), 1, ofiles[cursor_i]);
            fwrite(&Nb_XY.Ny, sizeof(short), 1, ofiles[cursor_i]);
            fwrite(&fx_lim.min, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&fx_lim.max, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&fy_lim.min, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&fy_lim.max, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&limits_f[col][cursor_i].min, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&limits_f[col][cursor_i].max, sizeof(double), 1, ofiles[cursor_i]);

            for (int ib = 0; ib < Nb_XY.Ny; ++ib) {
                for (int jb = 0; jb < Nb_XY.Nx; ++jb) {
                    fwrite(&Z[col][Nb_XY.Ny * ib + jb + Nb_XY.Nx * Nb_XY.Ny * (cursor_i)], sizeof(double), 1,
                           ofiles[cursor_i]);
                }
            }
            fclose(ofiles[cursor_i]);
        }
    }
}
void Converter::parallel_save_txt(size_t current_cursor, size_t col)
{
#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < obuff_size; ++i) {  // шаг по файлам
        size_t cursor_i = current_cursor + i;
        //  открытие и настройка файла
#ifdef _WIN32
        ofiles[cursor_i] = _wfopen(ofiles_names[all_files_cursor + i].wstring().c_str(), L"w");
#else
        ofiles[cursor_i] = fopen(ofiles_names[all_files_cursor + i].c_str(), "w");
#endif
        if (!ofiles[cursor_i]) {
            // TODO: добавить логи
        } else {
            std::vector<char> buffer(ofile_buff_size);
            setvbuf(ofiles[cursor_i], buffer.data(), _IOFBF, buffer.size());

            //------------------------------запись данных в файл---------------------------
            std::vector<char> writefile;
            size_t size = Nb_XY.Nx * Nb_XY.Ny * 32 + (Nb_XY.Nx - 1) * Nb_XY.Ny + Nb_XY.Ny + 256;

            writefile.reserve(size);
            char header[256];
            int header_len = snprintf(header, sizeof(header), "DSAA\n%d %d\n%.6f %.6f\n%.6f %.6f\n%.6f %.6f\n", Nb_XY.Nx,
                                      Nb_XY.Ny, fx_lim.min, fx_lim.max, fy_lim.min, fy_lim.max, limits_f[col][cursor_i].min,
                                      limits_f[col][cursor_i].max);
            if (header_len < 0 || header_len >= static_cast<int>(sizeof(header))) {
                // TODO: добавить логи
                fclose(ofiles[cursor_i]);
            } else {
                writefile.insert(writefile.end(), header, header + header_len);
                for (int ib = 0; ib < Nb_XY.Ny; ++ib) {
                    for (int jb = 0; jb < Nb_XY.Nx; ++jb) {
                        char value[32];
                        int len_value = snprintf(value, sizeof(value), "%.6f",
                                                 Z[col][Nb_XY.Ny * ib + jb + Nb_XY.Nx * Nb_XY.Ny * (i + current_cursor)]);
                        writefile.insert(writefile.end(), value, value + len_value);
                        if (jb < Nb_XY.Nx - 1) writefile.push_back(' ');
                    }
                    writefile.push_back('\n');
                }
                fwrite(writefile.data(), sizeof(char), writefile.size(), ofiles[cursor_i]);
            }
            fclose(ofiles[cursor_i]);
        }
    }
}
void Converter::consistent_save_bin(size_t current_cursor, size_t col)
{
    for (size_t i = 0; i < obuff_size; ++i) {  // шаг по файлам
        size_t cursor_i = current_cursor + i;
        // открытие и настройка файла
#ifdef _WIN32
        ofiles[cursor_i] = _wfopen(ofiles_names[all_files_cursor + i].wstring().c_str(), L"wb");
#else
        ofiles[cursor_i] = fopen(ofiles_names[all_files_cursor + i].c_str(), "wb");
#endif
        if (!ofiles[cursor_i]) {
            // TODO: добавить логи
        } else {
            std::vector<char> buffer(ofile_buff_size);
            setvbuf(ofiles[cursor_i], buffer.data(), _IOFBF, buffer.size());

            //------------------------------запись данных в файл---------------------------
            fwrite("DSBB", sizeof(char), 4, ofiles[cursor_i]);
            fwrite(&Nb_XY.Nx, sizeof(short), 1, ofiles[cursor_i]);
            fwrite(&Nb_XY.Ny, sizeof(short), 1, ofiles[cursor_i]);
            fwrite(&fx_lim.min, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&fx_lim.max, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&fy_lim.min, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&fy_lim.max, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&limits_f[col][cursor_i].min, sizeof(double), 1, ofiles[cursor_i]);
            fwrite(&limits_f[col][cursor_i].max, sizeof(double), 1, ofiles[cursor_i]);

            for (int ib = 0; ib < Nb_XY.Ny; ++ib) {
                for (int jb = 0; jb < Nb_XY.Nx; ++jb) {
                    float z = static_cast<float>(Z[col][Nb_XY.Ny * ib + jb + Nb_XY.Nx * Nb_XY.Ny * (cursor_i)]);
                    fwrite(&z, sizeof(float), 1, ofiles[cursor_i]);
                }
            }
            fclose(ofiles[cursor_i]);
        }
    }
}
void Converter::consistent_save_txt(size_t current_cursor, size_t col)
{
    for (size_t i = 0; i < obuff_size; ++i) {  // шаг по файлам
        size_t cursor_i = current_cursor + i;
        // открытие и настройка файла
#ifdef _WIN32
        ofiles[cursor_i] = _wfopen(ofiles_names[all_files_cursor + i].wstring().c_str(), L"wb");
#else
        ofiles[cursor_i] = fopen(ofiles_names[all_files_cursor + i].c_str(), "w");
#endif
        if (!ofiles[cursor_i]) {
            // TODO: добавить логи
        } else {
            std::vector<char> buffer(ofile_buff_size);
            setvbuf(ofiles[cursor_i], buffer.data(), _IOFBF, buffer.size());

            //------------------------------запись данных в файл---------------------------
            std::vector<char> writefile;
            size_t size = Nb_XY.Nx * Nb_XY.Ny * 32 + (Nb_XY.Nx - 1) * Nb_XY.Ny + Nb_XY.Ny + 256;

            writefile.reserve(size);
            char header[256];
            int header_len = snprintf(header, sizeof(header), "DSAA\n%d %d\n%.6f %.13f\n%.6f %.13f\n%.6f %.6f\n", Nb_XY.Nx,
                                      Nb_XY.Ny, fx_lim.min, fx_lim.max, fy_lim.min, fy_lim.max, limits_f[col][cursor_i].min,
                                      limits_f[col][cursor_i].max);
            if (header_len < 0 || header_len >= static_cast<int>(sizeof(header))) {
                // TODO: добавить логи
                fclose(ofiles[cursor_i]);
            } else {
                writefile.insert(writefile.end(), header, header + header_len);
                for (int ib = 0; ib < Nb_XY.Ny; ++ib) {
                    for (int jb = 0; jb < Nb_XY.Nx; ++jb) {
                        char value[32];
                        int len_value = snprintf(value, sizeof(value), "%.6f",
                                                 Z[col][Nb_XY.Ny * ib + jb + Nb_XY.Nx * Nb_XY.Ny * (i + current_cursor)]);
                        writefile.insert(writefile.end(), value, value + len_value);
                        if (jb < Nb_XY.Nx - 1) writefile.push_back(' ');
                    }
                    writefile.push_back('\n');
                }
                fwrite(writefile.data(), sizeof(char), writefile.size(), ofiles[cursor_i]);
            }
            fclose(ofiles[cursor_i]);
        }
    }
}
std::string Converter::extract_upper_axis(const std::string& input)  //"x (текст)" → "X", "y (любой текст)" → "Y"
{
    if (input.empty()) return "";

    // Ищем первую букву (a-z, A-Z)
    auto it = std::find_if(input.begin(), input.end(), [](unsigned char c) { return std::isalpha(c); });

    if (it == input.end()) return "";  // нет букв

    char c = std::toupper(static_cast<unsigned char>(*it));
    if (c != 'X' && c != 'Y' && c != 'Z') {
        return "";  // или throw, если нужно
    }

    return std::string(1, c);
};
void Converter::create_output_directory(std::string first_part, std::string second_part, std::string thrid_part,
                                        size_t files_size)
{
    //-----------------------создание поддиректории--------------------------
    std::filesystem::create_directory(output_directory / thrid_part);
    for (size_t i = 0; i < files_size; ++i) {
        ofiles_names.push_back(
            output_directory / thrid_part /
            (first_part + "_" + second_part + "_" + thrid_part + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
    }
}
Converter::ProjectionRefs Converter::get_projection()
{
    std::vector<double>& projection1 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для x
        if (xy->first == ParametrsList::X) {
            return data->get_x();
        } else if (xy->first == ParametrsList::Y) {
            return data->get_y();
        } else {
            return data->get_z();
        }
    }();

    std::vector<double>& projection2 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для y
        if (xy->second == ParametrsList::X) {
            return data->get_x();
        } else if (xy->second == ParametrsList::Y) {
            return data->get_y();
        } else {
            return data->get_z();
        }
    }();
    std::vector<double>& projection3 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для z
        if ((xy->first == ParametrsList::X && xy->second == ParametrsList::Y) ||
            (xy->first == ParametrsList::Y && xy->second == ParametrsList::X)) {
            return data->get_z();
        } else if ((xy->first == ParametrsList::X && xy->second == ParametrsList::Z) ||
                   (xy->first == ParametrsList::Z && xy->second == ParametrsList::X)) {
            return data->get_y();
        } else {
            return data->get_x();
        }
    }();

    std::vector<double>& v_projection1 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для x
        if (xy->first == ParametrsList::X) {
            return data->get_vx();
        } else if (xy->first == ParametrsList::Y) {
            return data->get_vy();
        } else {
            return data->get_vz();
        }
    }();

    std::vector<double>& v_projection2 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для y
        if (xy->second == ParametrsList::X) {
            return data->get_vx();
        } else if (xy->second == ParametrsList::Y) {
            return data->get_vy();
        } else {
            return data->get_vz();
        }
    }();
    return {std::ref(projection1), std::ref(projection2), std::ref(projection3), std::ref(v_projection1),
            std::ref(v_projection2)};
}