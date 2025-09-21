#pragma once
// #include <qtranslator.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#ifdef DEBUG
#include <iostream>
#endif
template <typename T>
class Translator
{
   private:
    static const std::array<std::string, 12>
        columns;  // массив заранее подготовленных колонок для выбора какие данные хранятся в txt
    std::vector<std::vector<T>> data;
    std::unordered_map<std::string_view, size_t> columns_list;  // хеш таблица c семантикой столбцов из txt файла
    std::vector<size_t> necessary_column;
    std::filesystem::path input_file_path;   // путь считывания txt
    std::filesystem::path output_file_path;  // путь сохранения grd
    std::ifstream input_file;                // поток считываемого файла
    std::ofstream output_file;               // поток выводимого файла
    std::pair<T, T> X_limits;                // first -min, second - max
    std::pair<T, T> Y_limits;                // first -min, second - max
    std::pair<T, T> Z_limits;                // first -min, second - max
    std::pair<int, int> Nxy;

    size_t column_in_file;  // число колонок в файле
    size_t N;

   public:
    Translator(const Translator&) = delete;             // отключение конструктора копирования
    Translator& operator=(const Translator&) = delete;  // запрещаем оператор присваивания копированием
    Translator(Translator&&) = delete;                  // запрещаем конструктор перемещения
    Translator& operator=(Translator&&) = delete;       // запрещаем опрератор присваивания перемещением

    Translator();
    ~Translator()
    {
        input_file.close();
        input_file.clear();
        output_file.close();
        output_file.clear();
        column_in_file = 0;
        N = 0;
    }

    void setup_columns(const std::initializer_list<std::string> columns_names,
                       const std::initializer_list<std::string> xyz_names);  // установка названий столбцов в виде ключей колонок

    void setup_columns(const std::vector<std::string> columns_names, const std::vector<std::string> xyz_names);

    void load_input_file(const std::filesystem::path inFileName);
    void load_output_file(const std::filesystem::path outFileName);
    void read_file();
    void translate_to_grd_bindings();
    void clear();
    void clear_output_file_state();

    const std::array<std::string, 12> get_columns() { return columns; };

    inline size_t get_column_count() { return column_in_file; };
    // проверка будет осуществлятся при вводе данных пользователем
    inline void setup_gridX(const std::pair<T, T> x_lim, const int Nx) noexcept
    {
        X_limits = x_lim;
        Nxy.first = Nx;
    }
    // проверка будет осуществлятся при вводе данных пользователем
    inline void setup_gridY(const std::pair<T, T> y_lim, const int Ny) noexcept
    {
        Y_limits = y_lim;
        Nxy.second = Ny;
    };

    // проверка будет осуществлятся при вводе данных пользователем
    inline void setup_limitsZ(const std::pair<T, T> z_lim) noexcept { Z_limits = z_lim; }

    // проверка будет осуществлятся при вводе данных пользователем
    inline void setup_gridXYZ(const std::pair<T, T> x_lim, const std::pair<T, T> y_lim, const std::pair<T, T> z_lim,
                              const std::pair<T, T> _Nxy) noexcept
    {
        X_limits = x_lim;
        Y_limits = y_lim;
        Z_limits = z_lim;
        Nxy = _Nxy;
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

template <class T>
const std::array<std::string, 12> Translator<T>::columns = {"x",  "y", "z",    "density", "vx",   "vy",
                                                            "vz", "e", "mass", "is_gas",  "None", "Termal"};
#include "Converter.tpp"
