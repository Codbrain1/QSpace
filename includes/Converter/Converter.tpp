#pragma once
#include <algorithm>
#include <charconv>
#include <cmath>
#include <codecvt>
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
Translator<T>::Translator() : X_limits(0, 0), Y_limits(0, 0), Nxy(0, 0), column_in_file(0), N(0)
{
}
template <class T>
void Translator<T>::load_output_file(const std::filesystem::path outFileName)
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
#else
    if (!std::filesystem::exists(dir_path) && std::string(dir_path.c_str()) != "") {
        throw std::filesystem::filesystem_error("this path not exist: " + std::string(outFileName.c_str()),
                                                std::make_error_code(std::errc::no_such_file_or_directory));
    }
    if (!std::filesystem::is_directory(dir_path) && std::string(dir_path.c_str()) != "") {
        throw std::filesystem::filesystem_error("this path not a directory:" + std::string(dir_path.c_str()),
                                                std::make_error_code(std::errc::not_a_directory));
    }
#endif
    // открываем выходной файл grd
    output_file_path = outFileName;
    output_file.open(outFileName);
#ifdef _WIN32
    if (!output_file.is_open()) throw std::runtime_error("file was not open: " + std::string(outFileName.string()));
#else
    if (!output_file.is_open()) throw std::runtime_error("file was not open: " + std::string(outFileName.c_str()));
#endif
}
template <class T>
void Translator<T>::load_input_file(const std::filesystem::path inFileName)
{
    // проверяем что существует входной файл
    if (!std::filesystem::exists(inFileName)) {
#ifdef _WIN32
        throw std::filesystem::filesystem_error("file not exist: " + std::string(inFileName.string()),
                                                std::make_error_code(std::errc::no_such_file_or_directory));
#else
        throw std::filesystem::filesystem_error("file not exist: " + std::string(inFileName.c_str()),
                                                std::make_error_code(std::errc::no_such_file_or_directory));
#endif
    }

    // открываем файл txt
    input_file.open(inFileName);
    // проверяем что файл открыт
#ifdef _WIN32
    if (!input_file.is_open()) throw std::runtime_error("file was not open: " + std::string(inFileName.string()));
#else
    if (!input_file.is_open()) throw std::runtime_error("file was not open: " + std::string(inFileName.c_str()));

#endif
    input_file_path = inFileName;
    input_file >> N;
    std::string temp;
    std::getline(input_file, temp);
    std::string line;
    if (!std::getline(input_file, line)) {
        input_file.close();
#ifdef _WIN32
        throw std::runtime_error("line was not read from " + std::string(inFileName.string()));
#else
        throw std::runtime_error("line was not read from " + std::string(inFileName.c_str()));
#endif
    }
    std::istringstream ss(line);

    // узнаем число колонок в файле
    T value;
    while (ss >> value) column_in_file++;
    input_file.close();
    // открываем входной файл в бинарном режиме
    input_file.open(inFileName, std::ios::binary | std::ios::ate);
    if (!input_file.is_open())
#ifdef _WIN32
        throw std::runtime_error("file was not open (binary state) from: " + std::string(inFileName.string()));
#else
        throw std::runtime_error("file was not open (binary state) from: " + std::string(inFileName.c_str()));
#endif
}
template <class T>
void Translator<T>::setup_columns(const std::initializer_list<std::string> columns_names,
                                  const std::initializer_list<std::string> xyz_names)
{
    // проверка на размеры списков инициализации
    if (columns_names.size() == 0 || xyz_names.size() == 0)
        throw std::invalid_argument("Translator<T> has 0 size 'columns names' or 'xyz names' list");
    if (columns_names.size() != column_in_file)
        throw std::invalid_argument("column list must contain elements == column in file");
    if (xyz_names.size() != 3) throw std::invalid_argument("xyz_names must contain elements == 3");

    // очистка старых значений
    data.clear();
    columns_list.clear();
    necessary_column.resize(column_in_file, std::size_t(-1));
    size_t size_data = 0;
    std::string_view value_column;

    for (auto& i : xyz_names) {
        // проверка что элемент допустим
        auto it = std::find(columns.begin(), columns.end(), i);
        // проверка что указанный элемент есть в списке колонок указанных пользователем
        auto it1 = std::find(columns_names.begin(), columns_names.end(), i);
        if (it == columns.end())
            throw std::invalid_argument("parametr: " + std::string(it->c_str()) + "not exist in parametrs list");
        if (it1 == columns_names.end())
            throw std::invalid_argument("parametr: " + std::string(it1->c_str()) + "not exist in parametrs list");

        size_t index = std::distance(columns_names.begin(), it1);
        if (size_data == 0) {
            columns_list.emplace(*std::find(columns.begin(), columns.end(), "x"), size_data);
        } else if (size_data == 1) {
            columns_list.emplace(*std::find(columns.begin(), columns.end(), "y"), size_data);
        } else {
            columns_list.emplace(*it, size_data);
        }
        necessary_column[index] = size_data;
        size_data++;
        if (size_data == 2) value_column = *it;
    }

    if (value_column == "Termal") {
        auto it = std::find(columns_names.begin(), columns_names.end(), "mass");
        auto it1 = std::find(columns_names.begin(), columns_names.end(), "Inner Energy");
        if (it == columns_names.end())
            throw std::invalid_argument("parametr: " + std::string(it->c_str()) + "not exist in parametrs list");
        if (it1 == columns_names.end())
            throw std::invalid_argument("parametr: " + std::string(it1->c_str()) + "not exist in parametrs list");
    }

    size_t count = 0;
    for (auto& i : columns_names) {
        auto it = std::find(columns.begin(), columns.end(), i);
        if (it == columns.end())
            throw std::invalid_argument("parametr: " + std::string(it->c_str()) + "not exist in parametrs list");
        if (value_column == "Termal") {
            throw std::runtime_error("in development");
            if (i == "mass") {
                columns_list.emplace(*it, size_data);
                necessary_column[count] = size_data;
                size_data++;
            } else if (i == "Inner Energy") {
                columns_list.emplace(*it, size_data);
                necessary_column[count] = size_data;
                size_data++;
            }
        }
        if (value_column == "is_gas") {
            columns_list.emplace(*it, size_data);
            necessary_column[count] = size_data;
            size_data++;
        }
        count++;
    }
    data.resize(size_data);
    for (auto& i : data) {
        i.reserve(N);
    }
}
template <class T>
void Translator<T>::setup_columns(const std::vector<std::string> columns_names, const std::vector<std::string> xyz_names)
{
    // проверка на размеры списков инициализации
    if (columns_names.size() == 0 || xyz_names.size() == 0)
        throw std::invalid_argument("Translator<T> has 0 size 'columns names' or 'xyz names' list");
    if (columns_names.size() != column_in_file)
        throw std::invalid_argument("column list must contain elements == column in file");
    if (xyz_names.size() != 3) throw std::invalid_argument("xyz_names must contain elements == 3");

    // очистка старых значений
    data.clear();
    columns_list.clear();
    necessary_column.resize(column_in_file, std::size_t(-1));
    size_t size_data = 0;
    std::string_view value_column;

    for (auto& i : xyz_names) {
        // проверка что элемент допустим
        auto it = std::find(columns.begin(), columns.end(), i);
        // проверка что указанный элемент есть в списке колонок указанных пользователем
        auto it1 = std::find(columns_names.begin(), columns_names.end(), i);
        if (it == columns.end())
            throw std::invalid_argument("parametr: " + std::string(it->c_str()) + "not exist in parametrs list");
        if (it1 == columns_names.end())
            throw std::invalid_argument("parametr: " + std::string(it1->c_str()) + "not exist in parametrs list");

        size_t index = std::distance(columns_names.begin(), it1);
        if (size_data == 0) {
            columns_list.emplace(*std::find(columns.begin(), columns.end(), "x"), size_data);
        } else if (size_data == 1) {
            columns_list.emplace(*std::find(columns.begin(), columns.end(), "y"), size_data);
        } else {
            columns_list.emplace(*it, size_data);
        }
        necessary_column[index] = size_data;
        size_data++;
        if (size_data == 2) value_column = *it;
    }

    if (value_column == "Termal") {
        auto it = std::find(columns_names.begin(), columns_names.end(), "mass");
        auto it1 = std::find(columns_names.begin(), columns_names.end(), "Inner Energy");
        if (it == columns_names.end())
            throw std::invalid_argument("parametr: " + std::string(it->c_str()) + "not exist in parametrs list");
        if (it1 == columns_names.end())
            throw std::invalid_argument("parametr: " + std::string(it1->c_str()) + "not exist in parametrs list");
    }

    size_t count = 0;
    for (auto& i : columns_names) {
        auto it = std::find(columns.begin(), columns.end(), i);
        if (it == columns.end())
            throw std::invalid_argument("parametr: " + std::string(i.c_str()) + "not exist in parametrs list");
        if (value_column == "Termal") {
            throw std::runtime_error("in development");
            if (i == "mass") {
                columns_list.emplace(*it, size_data);
                necessary_column[count] = size_data;
                size_data++;
            } else if (i == "Inner Energy") {
                columns_list.emplace(*it, size_data);
                necessary_column[count] = size_data;
                size_data++;
            }
        }
        if (value_column == "is_gas") {
            columns_list.emplace(*it, size_data);
            necessary_column[count] = size_data;
            size_data++;
        }
        count++;
    }
    data.resize(size_data);
    for (auto& i : data) {
        i.reserve(N);
    }
}
template <class T>
void Translator<T>::read_file()
{
    // чтение текстового формата файла
    std::streamsize size = input_file.tellg();  // загружаем сразу весь файл в оперативную память
    std::string content(size, '\0');            // парсим файл в string
    input_file.seekg(0, std::ios::beg);         // устанавливаем курсор на начало файла
    input_file.read(content.data(), size);      // читаем файл в RAM

    size_t _N;
    const char* content_ptr = content.c_str();                    // указатель на начало content
    const char* content_end = content_ptr + content.size();       // указатель на конец content
    auto result = std::from_chars(content_ptr, content_end, _N);  // парсин числового значения из content
    // если данные не были распаршены
    if (result.ec != std::errc() || _N != N) {
        throw std::runtime_error("Invalid N in file");
    }
    // пропускаем \n
    content_ptr = std::strchr(content_ptr, '\n');

    if (!content_ptr) throw std::runtime_error("No newline after N");
    content_ptr++;

    size_t n = 0;
    // цикл выполныется пока весь content не спаршен или пока на закончились значимые строки
    while (content_ptr < content_end && n < N) {
        // перебираем все столбцы
        for (size_t i = 0; i < column_in_file; ++i) {
            while (content_ptr < content_end && (*content_ptr == ' ' || *content_ptr == '\t'))
                ++content_ptr;  // пока встечаются мусорные знаки

            if (content_ptr >= content_end)  // если файл не содержит ничего кроме числа строк то ошибка
#ifdef _WIN32
                throw std::runtime_error("unexpected end file: " + std::string(input_file_path.string()));
#else
                throw std::runtime_error("unexpected end file: " + std::string(input_file_path.c_str()));
#endif
            T value;
            result = std::from_chars(content_ptr, content_end, value);  // парсинг значения
            if (result.ec != std::errc())
#ifdef _WIN32
                throw std::runtime_error("Parse error in file:" + std::string(input_file_path.string()) + " at row " +
                                         std::to_string(n));
#else
                throw std::runtime_error("Parse error in file:" + std::string(input_file_path.c_str()) + " at row " +
                                         std::to_string(n));
#endif
            content_ptr = result.ptr;

            size_t index = necessary_column[i];
            if (index != size_t(-1)) {
                data[index].push_back(value);  // запись значения в массив
            }
        }
        while (content_ptr < content_end && *content_ptr != '\n') ++content_ptr;
        if (content_ptr < content_end) ++content_ptr;  // За '\n'
        n++;
    }
}

template <class T>
void Translator<T>::translate_to_grd_bindings()
{
#ifdef DEBUG
    print();
#endif
    const auto& x = data[columns_list.at("x")];
    const auto& y = data[columns_list.at("y")];
    const auto& z = data[2];

    std::vector<std::pair<T, size_t>> Z(Nxy.first * Nxy.second, std::pair<T, size_t>(0, size_t(0)));
    T z_min = std::numeric_limits<T>::max();
    T z_max = std::numeric_limits<T>::min();
    if (columns_list.find("density") != columns_list.end()) {
        const T dx = (X_limits.second - X_limits.first) / (Nxy.first);
        const T dy = (Y_limits.second - Y_limits.first) / (Nxy.second);
        // const T x_0 = X_limits.first;
        // const T y_0 = Y_limits.first;
        auto it = columns_list.find("is_gas");

        if (it != columns_list.end()) {  // если есть столбец is_gas
            const auto& is_gas = data[columns_list.at("is_gas")];
            for (size_t k = 0; k < N; ++k) {
                if (is_gas[k] != 0) {
                    int col = static_cast<int>(std::round((x[k] - X_limits.first) / (dx - 1)));
                    int row = static_cast<int>(std::round((y[k] - Y_limits.first) / (dy - 1)));
                    if (col >= 0 && col < Nxy.first && row >= 0 && row < Nxy.second) {
                        size_t index = row * Nxy.first + col;
                        Z[index].first += z[k];
                        Z[index].second += 1;
                        z_min = std::min(z_min, z[k]);
                        z_max = std::max(z_max, z[k]);
                    }
                }
            }
        } else {  // если нет столбца is_gas
            for (size_t k = 0; k < N; ++k) {
                int col = static_cast<int>(std::round((x[k] - X_limits.first) / dx));
                int row = static_cast<int>(std::round((y[k] - Y_limits.first) / dy));
                if (col >= 0 && col < Nxy.first && row >= 0 && row < Nxy.second) {
                    size_t index = row * Nxy.first + col;
                    Z[index].first += z[k];
                    Z[index].second += 1;
                    z_min = std::min(z_min, z[k]);
                    z_max = std::max(z_max, z[k]);
                }
            }
        }
    } else if (columns_list.find("Termal") != columns_list.end()) {
        throw std::runtime_error("in developmant");
    }
    // усреднение значений
    const T NoData = static_cast<T>(1.70141e+38);
    for (size_t i = 0; i < Z.size(); ++i) {
        if (Z[i].second > 0)
            Z[i].first /= Z[i].second;
        else
            Z[i].first = NoData;
    }
    Z_limits.first = z_min;
    Z_limits.second = z_max;
    // запись данных в файл
    output_file << "DSAA\n";
    output_file << Nxy.first << " " << Nxy.second << std::endl;
    output_file << std::setprecision(10) << X_limits.first << " " << X_limits.second << "\n"
                << Y_limits.first << " " << Y_limits.second << "\n"
                << Z_limits.first << " " << Z_limits.second << "\n";

    for (int i = 0; i < Nxy.first; ++i) {
        for (int j = 0; j < Nxy.second; ++j) {
            size_t index = i * Nxy.first + j;
            output_file << std::setprecision(10) << Z[index].first << " ";
        }
        output_file << "\n";
    }
}

template <class T>
void Translator<T>::clear()
{
    data.clear();
    columns_list.clear();
    necessary_column.clear();
    input_file_path = std::filesystem::path();
    output_file_path = std::filesystem::path();
    input_file.close();
    input_file.clear();
    output_file.close();
    output_file.clear();
    X_limits = std::pair<T, T>(0, 0);
    Y_limits = std::pair<T, T>(0, 0);
    Z_limits = std::pair<T, T>(0, 0);
    Nxy = std::pair<T, T>(0, 0);
    column_in_file = 0;
    N = 0;
}
template <class T>
void Translator<T>::clear_output_file_state()
{
    output_file.close();
    output_file.clear();
    columns_list.clear();
    data.clear();
    necessary_column.clear();
    X_limits = std::pair<T, T>(0, 0);
    Y_limits = std::pair<T, T>(0, 0);
    Z_limits = std::pair<T, T>(0, 0);
}
