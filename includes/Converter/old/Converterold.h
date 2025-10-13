#pragma once
// #include <qtranslator.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#ifdef DEBUG
#include <iostream>
#endif
template <typename T>
class Converter
{
   private:
    static constexpr std::array<std::string_view, 13> columns_accept_FIELD = {
        "x", "y",    "z",      "density", "vx",     "vy",          "vz",
        "e", "mass", "is_gas", "None",    "Termal", "Inner Energy"};  // массив заранее
                                                                      // подготовленных колонок для
                                                                      // выбора какие данные
                                                                      // хранятся в txt
    static constexpr std::string_view find_value(std::string_view value)
    {
        auto it = std::find(columns_accept_FIELD.begin(), columns_accept_FIELD.end(), value);
        return it != columns_accept_FIELD.end() ? *it : std::string_view{};
    }
    static constexpr std::array<std::string_view, 3> xy_allowed_FIELD = {find_value("x"), find_value("y"), find_value("x")};
    static constexpr std::array<std::string_view, 3> z_allowed_FIELD = {find_value("z"), find_value("density"),
                                                                        find_value("Termal")};

    std::vector<std::vector<T>> data_FIELD;
    std::unordered_map<std::string_view, size_t> columns_list_FIELD;  // хеш таблица c семантикой столбцов из txt файла
    std::vector<size_t> necessary_column_FIELD;                       // номера необходимых колонок
    std::filesystem::path input_file_path_FIELD;                      // путь считывания txt
    std::filesystem::path output_file_path_FIELD;                     // путь сохранения grd
    std::ifstream input_file_FIELD;                                   // поток считываемого файла
    std::ofstream output_file_FIELD;                                  // поток выводимого файла
    std::pair<T, T> X_limits_FIELD;                                   // first -min, second - max
    std::pair<T, T> Y_limits_FIELD;                                   // first -min, second - max
    std::pair<T, T> Z_limits_FIELD;                                   // first -min, second - max
    std::pair<int, int> Nxy_FIELD;
    bool Z_exist_in_file_FIELD;
    size_t count_column_FIELD;  // число колонок в файле
    size_t N_FIELD;             // число частиц(строк) записанных в файле

   public:
    Converter(const Converter&) = delete;             // отключение конструктора копирования
    Converter& operator=(const Converter&) = delete;  // запрещаем оператор присваивания копированием
    Converter(Converter&&) noexcept;                  // конструктор перемещения
    Converter& operator=(Converter&&);                // запрещаем опрератор присваивания перемещением

    Converter();
    ~Converter()
    {
        input_file_FIELD.close();
        input_file_FIELD.clear();
        output_file_FIELD.close();
        output_file_FIELD.clear();
        count_column_FIELD = 0;
        N_FIELD = 0;
    }

    void setup_columns(const std::vector<std::string> columns_names,
                       const std::vector<std::string> xyz_names);  // установка названий столбцов в виде ключей колонок

    void load_input_file(const std::filesystem::path inFileName);    // загрузка данных о входном файле
    void load_output_file(const std::filesystem::path outFileName);  // загрузка данных о выходном файле
    void read_input_file();                                          // чтение входного файла
    void translate_to_grd_bindings();  // запись данных в выходной файл (методом попадания частицы в
                                       // ячейку)
    void clear();                      // очистка данных
    void clear_output_file_state();    // очистка данных о выходном файле
    void close_input_file()
    {
        if (input_file_FIELD.is_open()) input_file_FIELD.close();
    }
    static constexpr std::array<std::string_view, 13> get_columns()
    {
        return columns_accept_FIELD;
    };  // возвращает допустимые колонки

    inline size_t get_column_count() { return count_column_FIELD; };  // возвращает число колонок в входном файле
    // проверка будет осуществлятся при вводе данных пользователем
    inline void setup_gridX(const std::pair<T, T> x_lim,
                            const int Nx) noexcept  // установка x сетки
    {
        X_limits_FIELD = x_lim;
        Nxy_FIELD.first = Nx;
    }
    // проверка будет осуществлятся при вводе данных пользователем
    inline void setup_gridY(const std::pair<T, T> y_lim,
                            const int Ny) noexcept  // установка y сетки
    {
        Y_limits_FIELD = y_lim;
        Nxy_FIELD.second = Ny;
    };

    // проверка будет осуществлятся при вводе данных пользователем
    inline void setup_limitsZ(const std::pair<T, T> z_lim) noexcept { Z_limits_FIELD = z_lim; }  // задание пределов по z

    // проверка будет осуществлятся при вводе данных пользователем
    inline void setup_gridXYZ(const std::pair<T, T> x_lim, const std::pair<T, T> y_lim, const std::pair<T, T> z_lim,
                              const std::pair<T, T> _Nxy) noexcept
    {
        X_limits_FIELD = x_lim;
        Y_limits_FIELD = y_lim;
        Z_limits_FIELD = z_lim;
        Nxy_FIELD = _Nxy;
    };

#ifdef DEBUG
    // отладочные функции
    //=========================================
    void print()
    {
        for (auto j : columns_list) std::cout << j.first << "\t";
        std::cout << "\n";
        for (int i = 0; i < N; ++i) {
            for (auto j : columns_list) std::cout << j.first << " " << j.second << " " << data[j.second][i] << "\t";
            std::cout << "\n";
        }
    }
#endif
};

#include "Converter.tpp"
