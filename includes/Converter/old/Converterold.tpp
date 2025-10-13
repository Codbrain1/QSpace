#pragma once
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <codecvt>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "Converter.h"
template <class T>
Converter<T>::Converter() : X_limits_FIELD(0, 0), Y_limits_FIELD(0, 0), Nxy_FIELD(0, 0), count_column_FIELD(0), N_FIELD(0)
{
}
template <class T>
void Converter<T>::load_output_file(const std::filesystem::path outFileName)
{
    // вытаскиваем родительскую директорию выходного файла проверяем что она существует
    auto dir_path = outFileName.parent_path();
#ifdef _WIN32
    if (!std::filesystem::exists(dir_path) && std::string(dir_path.string()) != "") {
        throw std::filesystem::filesystem_error("this path not exist: " + std::string(outFileName.string()),
                                                std::make_error_code(std::errc::no_such_file_or_directory));
    }
    if (!std::filesystem::is_directory(dir_path) && std::string(dir_path.string()) != "") {
        throw std::filesystem::filesystem_error("this path not a directory:" + std::string(dir_path.string()),
                                                std::make_error_code(std::errc::not_a_directory));
    }
#else  // если система linux
    if (!std::filesystem::exists(dir_path) && std::string(dir_path.c_str()) != "") {  // проверка на существование
                                                                                      // введенного пути
        throw std::filesystem::filesystem_error("this path not exist: " + std::string(outFileName.c_str()),
                                                std::make_error_code(std::errc::no_such_file_or_directory));
    }
    if (!std::filesystem::is_directory(dir_path) && std::string(dir_path.c_str()) != "") {  // проверка что указанный
                                                                                            // путь не является путем к
                                                                                            // директории
        throw std::filesystem::filesystem_error("this path not a directory:" + std::string(dir_path.c_str()),
                                                std::make_error_code(std::errc::not_a_directory));
    }
#endif
    // открываем выходной файл grd
    output_file_path_FIELD = outFileName;
    output_file_FIELD.open(outFileName);

    // проверка что файл был успешно открыт
#ifdef _WIN32
    if (!output_file_FIELD.is_open()) throw std::runtime_error("file was not open: " + std::string(outFileName.string()));
#else
    if (!output_file_FIELD.is_open()) throw std::runtime_error("file was not open: " + std::string(outFileName.c_str()));
#endif
}
template <class T>
void Converter<T>::load_input_file(const std::filesystem::path inFileName)
{
    // проверяем что существует входной файл
    if (!std::filesystem::exists(inFileName)) {
#ifdef _WIN32
        throw std::filesystem::filesystem_error("file not exist: " + std::string(inFileName.string()),
                                                std::make_error_code(std::errc::no_such_file_or_directory));
#else  // елси система линукс
        throw std::filesystem::filesystem_error("file not exist: " + std::string(inFileName.c_str()),
                                                std::make_error_code(std::errc::no_such_file_or_directory));
#endif
    }

    // открываем файл txt
    input_file_FIELD.open(inFileName);
    // проверяем что файл открыт
#ifdef _WIN32
    if (!input_file_FIELD.is_open()) throw std::runtime_error("file was not open: " + std::string(inFileName.string()));
#else  // елси система линукс
    if (!input_file_FIELD.is_open()) throw std::runtime_error("file was not open: " + std::string(inFileName.c_str()));

#endif
    input_file_path_FIELD = inFileName;
    input_file_FIELD >> N_FIELD;  // считываем число строк в файле(число частиц)
    std::string temp;
    std::getline(input_file_FIELD, temp);
    std::string line;
    if (!std::getline(input_file_FIELD, line)) {
        input_file_FIELD.close();

#ifdef _WIN32
        throw std::runtime_error("line was not read from " + std::string(inFileName.string()));
#else
        throw std::runtime_error("line was not read from " + std::string(inFileName.c_str()));
#endif
    }

    std::istringstream ss(line);

    // узнаем число колонок в файле
    T value;
    while (ss >> value) count_column_FIELD++;
    input_file_FIELD.close();
    // открываем входной файл в бинарном режиме
    input_file_FIELD.open(inFileName, std::ios::binary | std::ios::ate);
    if (!input_file_FIELD.is_open())
#ifdef _WIN32
        throw std::runtime_error("file was not open (binary state) from: " + std::string(inFileName.string()));
#else
        throw std::runtime_error("file was not open (binary state) from: " + std::string(inFileName.c_str()));
#endif
}
template <class T>
void Converter<T>::setup_columns(const std::vector<std::string> columns_names, const std::vector<std::string> xyz_names)
{
    // проверка на размеры списков инициализации
    if (columns_names.size() == 0 || xyz_names.size() == 0)
        throw std::invalid_argument("Translator<T> has 0 size 'columns names' or 'xyz names'list");
    if (columns_names.size() != count_column_FIELD)
        throw std::invalid_argument("column list mustcontain elements == column in file");
    if (xyz_names.size() != 3) throw std::invalid_argument("xyz_names must contain elements == 3");

    // очистка старых значений
    data_FIELD.clear();
    columns_list_FIELD.clear();
    necessary_column_FIELD.resize(count_column_FIELD, std::size_t(-1));

    size_t size_data = 0;  // отвечает за номер необходимого столбца в хеш таблице
    // добавление X оси
    size_t index = -1;
    auto X_it = std::find(xy_allowed_FIELD.begin(), xy_allowed_FIELD.begin(), xyz_names[0]);
    auto X_it1 = std::find(columns_names.begin(), columns_names.end(), xyz_names[0]);

    if (X_it == xy_allowed_FIELD.end())
        throw std::invalid_argument("parametr: " + xyz_names[0] + " notexist in parametrs list");
    if (X_it1 == columns_names.end())
        throw std::invalid_argument("parametr: " + xyz_names[0] + "not exist in parametrs list");
    // добавляем номер колонки абсцисс (X) в файле, вхешь таблицу
    // ключ "x", значение 0 (индекс)
    columns_list_FIELD.emplace(*std::find(columns_accept_FIELD.begin(), columns_accept_FIELD.end(), "x"), size_data);
    index = std::distance(columns_names.begin(), X_it1);  // поиск номера колонки снужным названием
    necessary_column_FIELD[index] = size_data;            // задаем индекс 0
    size_data++;

    // добавление Y оси
    auto Y_it = std::find(xy_allowed_FIELD.begin(), xy_allowed_FIELD.begin(), xyz_names[1]);
    auto Y_it1 = std::find(columns_names.begin(), columns_names.end(), xyz_names[1]);

    if (Y_it == xy_allowed_FIELD.end())
        throw std::invalid_argument("parametr: " + xyz_names[1] + " notexist in parametrs list");
    if (Y_it1 == columns_names.end())
        throw std::invalid_argument("parametr: " + xyz_names[1] + "not exist in parametrs list");

    // добавляем номер колонки абсцисс (Y) в файле, в хеш таблицу
    // ключ x, значение 1 (индекс)
    columns_list_FIELD.emplace(*std::find(columns_accept_FIELD.begin(), columns_accept_FIELD.end(), "y"), size_data);
    index = std::distance(columns_names.begin(), Y_it1);  // поиск номера колонки снужным названием
    necessary_column_FIELD[index] = size_data;            // задаем индекс 1
    size_data++;

    // добавление оси Z
    auto Z_it = std::find(z_allowed_FIELD.begin(), z_allowed_FIELD.end(), xyz_names[2]);
    auto Z_it1 = std::find(columns_names.begin(), columns_names.end(), xyz_names[2]);

    if (Z_it == z_allowed_FIELD.end())
        throw std::invalid_argument("parametr: " + xyz_names[2] + " notexist in parametrs list");

    if (xyz_names[2] == "Termal") {  // если требуется записать значения Температуры
        columns_list_FIELD.emplace(*std::find(columns_accept_FIELD.begin(), columns_accept_FIELD.end(), xyz_names[2]),
                                   size_data);
        // должен быть указан столбец массы auto
        auto it_mass = std::find(columns_names.begin(), columns_names.end(), "mass");
        // должен быть указан столбец внутренней энергии
        auto it_inner_energy = std::find(columns_names.begin(), columns_names.end(), "Inner Energy");
        if (it_mass == columns_names.end()) throw std::invalid_argument("parametr: mass not exist inparametrs list");
        if (it_inner_energy == columns_names.end())
            throw std::invalid_argument("parametr: Inner Energy not exist in parametrs list");

        // проверка содержится ли данные о темпиратуре в файле
        if (Z_it1 == columns_names.end()) {  // считывание из файла если столбец не указан
            Z_exist_in_file_FIELD = false;
        } else {
            Z_exist_in_file_FIELD = true;
            index = std::distance(columns_names.begin(), Z_it1);  // поиск номера колонки снужным названием
            necessary_column_FIELD[index] = size_data;            // задаем индекс 1
        }
    } else {
        if (Z_it1 == columns_names.end())  // проверка содержится ли данные о темпиратуре в файле
            throw std::invalid_argument("parametr: " + xyz_names[2] + " not exist in parametrs list");

        Z_exist_in_file_FIELD = true;
        index = std::distance(columns_names.begin(), Z_it1);  // поиск номера колонки снужным названием
        necessary_column_FIELD[index] = size_data;
        columns_list_FIELD.emplace(*std::find(columns_accept_FIELD.begin(), columns_accept_FIELD.end(), xyz_names[2]),
                                   size_data);
    }
    size_data++;

    size_t count = 0;
    for (auto& i : columns_names) {
        auto it = std::find(columns_accept_FIELD.begin(), columns_accept_FIELD.end(), i);
        if (it == columns_accept_FIELD.end()) throw std::invalid_argument("parametr: " + i + "not exist inparametrs list");
        if (xyz_names[2] == "Termal") {
            if (i == "mass") {
                columns_list_FIELD.emplace(*it, size_data);
                necessary_column_FIELD[count] = size_data;
                size_data++;
            } else if (i == "Inner Energy") {
                columns_list_FIELD.emplace(*it, size_data);
                necessary_column_FIELD[count] = size_data;
                size_data++;
            }
        }
        if (i == "is_gas") {
            columns_list_FIELD.emplace(*it, size_data);
            necessary_column_FIELD[count] = size_data;
            size_data++;
        }
        count++;
    }
    data_FIELD.resize(size_data);
    for (auto& i : data_FIELD) {
        i.reserve(N_FIELD);
    }
}
template <class T>
void Converter<T>::read_input_file()
{
    // чтение текстового формата файла
    std::streamsize size = input_file_FIELD.tellg();  // загружаем сразу весь файл в оперативную память
    std::string content(size, '\0');                  // парсим файл в string
    input_file_FIELD.seekg(0, std::ios::beg);         // устанавливаем курсор на начало файла
    input_file_FIELD.read(content.data(), size);      // читаем файл в RAM

    size_t _N;
    const char* content_ptr = content.c_str();                    // указатель на начало content
    const char* content_end = content_ptr + content.size();       // указатель на конец content
    auto result = std::from_chars(content_ptr, content_end, _N);  // парсинг числового значения из content
    // если данные не были распаршены
    if (result.ec != std::errc() || _N != N_FIELD) {
        throw std::runtime_error("Invalid N in file");
    }
    // пропускаем \n
    content_ptr = std::strchr(content_ptr, '\n');

    if (!content_ptr) throw std::runtime_error("No newline after N");
    content_ptr++;

    size_t n = 0;
    // цикл выполняется пока весь content не спаршен или пока на закончились значимые строки
    while (content_ptr < content_end && n < N_FIELD) {
        // перебираем все столбцы
        for (size_t i = 0; i < count_column_FIELD; ++i) {
            while (content_ptr < content_end && (*content_ptr == ' ' || *content_ptr == '\t'))
                ++content_ptr;  // пока встечаются мусорные знаки

            if (content_ptr >= content_end)  // если файл не содержит ничего кроме числа строк то ошибка
#ifdef _WIN32
                throw std::runtime_error("unexpected end file: " + std::string(input_file_path_FIELD.string()));
#else
                throw std::runtime_error("unexpected end file: " + std::string(input_file_path_FIELD.c_str()));
#endif
            T value;
            result = std::from_chars(content_ptr, content_end, value);  // парсинг значения
            if (result.ec != std::errc())
#ifdef _WIN32
                throw std::runtime_error("Parse error in file:" + std::string(input_file_path_FIELD.string()) + " at row " +
                                         std::to_string(n));
#else
                throw std::runtime_error("Parse error in file:" + std::string(input_file_path_FIELD.c_str()) + " at row " +
                                         std::to_string(n));
#endif
            content_ptr = result.ptr;

            size_t index = necessary_column_FIELD[i];
            if (index != size_t(-1)) {
                data_FIELD[index].push_back(value);  // запись значения в массив
            }
        }
        while (content_ptr < content_end && *content_ptr != '\n') ++content_ptr;
        if (content_ptr < content_end) ++content_ptr;  // За '\n'
        n++;
    }
}

template <class T>
void Converter<T>::translate_to_grd_bindings()
{
#ifdef DEBUG
    print();
#endif
    const auto& x = data_FIELD[columns_list_FIELD.at("x")];
    const auto& y = data_FIELD[columns_list_FIELD.at("y")];
    const auto& z = data_FIELD[2];

    std::vector<std::pair<T, unsigned long int>> Z(Nxy_FIELD.first * Nxy_FIELD.second, std::pair<T, size_t>(0, size_t(0)));
    T z_min = std::numeric_limits<T>::max();
    T z_max = std::numeric_limits<T>::min();
    auto it = columns_list_FIELD.find("is_gas");

    if (it != columns_list_FIELD.end()) {  // если есть столбец is_gas

        const auto& is_gas = data_FIELD[columns_list_FIELD.at("is_gas")];
        const T dx = (X_limits_FIELD.second - X_limits_FIELD.first) / (Nxy_FIELD.first);
        const T dy = (Y_limits_FIELD.second - Y_limits_FIELD.first) / (Nxy_FIELD.second);
        // const T x_0 = X_limits.first;
        // const T y_0 = Y_limits.first;

        if (columns_list_FIELD.find("Termal") != columns_list_FIELD.end() && !Z_exist_in_file_FIELD) {
            const auto& mass = data_FIELD[columns_list_FIELD.at("mass")];
            const auto& inner_energy = data_FIELD[columns_list_FIELD.at("Inner Energy")];

            // подсчет темпeратуры
        }

        for (size_t k = 0; k < N_FIELD; ++k) {
            if (is_gas[k] != 0) {
                int col = static_cast<int>(std::floor((x[k] - X_limits_FIELD.first) / dx));
                int row = static_cast<int>(std::floor((y[k] - Y_limits_FIELD.first) / dy));
                if (col >= 0 && col < Nxy_FIELD.first && row >= 0 && row < Nxy_FIELD.second) {
                    size_t index = row * Nxy_FIELD.first + col;
                    Z[index].first += z[k];
                    Z[index].second += 1;
                    z_min = std::min(z_min, z[k]);
                    z_max = std::max(z_max, z[k]);
                }
            }
        }
    } else {  // если нет столбца is_gas
        const T dx = (X_limits_FIELD.second - X_limits_FIELD.first) / (Nxy_FIELD.first);
        const T dy = (Y_limits_FIELD.second - Y_limits_FIELD.first) / (Nxy_FIELD.second);
        // const T x_0 = X_limits.first;
        // const T y_0 = Y_limits.first;
        if (columns_list_FIELD.find("Termal") != columns_list_FIELD.end() && !Z_exist_in_file_FIELD) {
            const auto& mass = data_FIELD[columns_list_FIELD.at("mass")];
            const auto& inner_energy = data_FIELD[columns_list_FIELD.at("Inner Energy")];
            // подсчет температуры
            // TODO: реализовать подсчет температры
        }
        for (size_t k = 0; k < N_FIELD; ++k) {
            int col = static_cast<int>(std::floor((x[k] - X_limits_FIELD.first) / dx));
            int row = static_cast<int>(std::floor((y[k] - Y_limits_FIELD.first) / dy));
            if (col >= 0 && col < Nxy_FIELD.first && row >= 0 && row < Nxy_FIELD.second) {
                size_t index = row * Nxy_FIELD.first + col;
                Z[index].first += z[k];
                Z[index].second += 1;
                z_min = std::min(z_min, z[k]);
                z_max = std::max(z_max, z[k]);
            }
        }
    }
    // усреднение значений
    const T NoData = static_cast<T>(1.70141e+38);
    for (size_t i = 0; i < Z.size(); ++i) {
        if (Z[i].second > 0)
            Z[i].first /= Z[i].second;
        else
            Z[i].first = NoData;
    }
    Z_limits_FIELD.first = z_min;
    Z_limits_FIELD.second = z_max;
    // запись данных в файл
    output_file_FIELD << "DSAA\n";
    output_file_FIELD << Nxy_FIELD.first << " " << Nxy_FIELD.second << std::endl;
    output_file_FIELD << std::setprecision(10) << X_limits_FIELD.first << " " << X_limits_FIELD.second << "\n"
                      << Y_limits_FIELD.first << " " << Y_limits_FIELD.second << "\n"
                      << Z_limits_FIELD.first << " " << Z_limits_FIELD.second << "\n";

    for (int j = 0; j < Nxy_FIELD.second; ++j) {
        for (int i = 0; i < Nxy_FIELD.first; ++i) {
            size_t index = j * Nxy_FIELD.first + i;
            output_file_FIELD << std::setprecision(10) << Z[index].first << " ";
        }
        output_file_FIELD << "\n";
    }
}
template <class T>
Converter<T>::Converter(Converter&& other) noexcept
    : data_FIELD(std::move(other.data_FIELD)),
      columns_list_FIELD(std::move(other.columns_list_FIELD)),
      necessary_column_FIELD(std::move(other.necessary_column_FIELD)),
      input_file_path_FIELD(std::move(other.input_file_path_FIELD)),
      output_file_path_FIELD(std::move(other.output_file_path_FIELD)),
      input_file_FIELD(std::move(other.input_file_FIELD)),
      output_file_FIELD(std::move(other.output_file_FIELD)),
      X_limits_FIELD(std::move(other.X_limits_FIELD)),
      Y_limits_FIELD(std::move(other.Y_limits_FIELD)),
      Z_limits_FIELD(std::move(other.Z_limits_FIELD)),
      Nxy_FIELD(std::move(other.Nxy_FIELD)),
      Z_exist_in_file_FIELD(other.Z_exist_in_file_FIELD),
      count_column_FIELD(other.count_column_FIELD),
      N_FIELD(other.N_FIELD)
{
}
template <class T>
void Converter<T>::clear()
{
    data_FIELD.clear();
    columns_list_FIELD.clear();
    necessary_column_FIELD.clear();
    input_file_path_FIELD = std::filesystem::path();
    output_file_path_FIELD = std::filesystem::path();
    input_file_FIELD.close();
    input_file_FIELD.clear();
    output_file_FIELD.close();
    output_file_FIELD.clear();
    X_limits_FIELD = std::pair<T, T>(0, 0);
    Y_limits_FIELD = std::pair<T, T>(0, 0);
    Z_limits_FIELD = std::pair<T, T>(0, 0);
    Nxy_FIELD = std::pair<T, T>(0, 0);
    count_column_FIELD = 0;
    N_FIELD = 0;
}
template <class T>
void Converter<T>::clear_output_file_state()
{
    output_file_FIELD.close();
    output_file_FIELD.clear();
    columns_list_FIELD.clear();
    data_FIELD.clear();
    necessary_column_FIELD.clear();
    X_limits_FIELD = std::pair<T, T>(0, 0);
    Y_limits_FIELD = std::pair<T, T>(0, 0);
    Z_limits_FIELD = std::pair<T, T>(0, 0);
}
