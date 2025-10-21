#include <charconv>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <system_error>
#include <variant>
#include <vector>

#include "Converter/DataStorage.h"
#include "DataStorageVisit.cpp"
DataStorage::DataStorage(const std::vector<std::filesystem::path>& pathes)
    : ibuff_size(8), current_cursor(0), ifile_buffer_size(16384), capacity(0)
{
    if (!pathes.empty()) {
        ifile_names.reserve(pathes.size());
        for (const auto& file_path : pathes) {  // проверяем что все пути существуют
            if (std::filesystem::exists(file_path)) {
                ifile_names.push_back(file_path);
            } else {
                // TODO: добавить класс logov
            }
        }
        if (ibuff_size > ifile_names.size()) ibuff_size = ifile_names.size();
        ifiles.reserve(ibuff_size);
        ifiles.assign(ibuff_size, nullptr);
        Ns.reserve(ifile_names.size());
    }
}
void DataStorage::load_file_metadate_txt()
{
    for (size_t i = 0; i < ifile_names.size(); ++i) {
        FILE* file = nullptr;
#ifdef _WIN32
        file = _wfopen(ifile_names[i].wstring(), L"r");
#else
        file = fopen(ifile_names[i].c_str(), "r");
#endif
        if (file != nullptr) {
            int N = 0;
            fscanf(file, "%d", &N);
            if (N > 0) {
                Ns.push_back(N);
            } else {
                throw std::runtime_error("invalid content in file: " + ifile_names[i].string());
            }
        } else {
            throw std::runtime_error("file: " + ifile_names[i].string() + "was not open");
        }
    }
}
void DataStorage::load_file_metadate_bin()
{
    for (size_t i = 0; i < ifile_names.size(); ++i) {
        FILE* file = nullptr;
#ifdef _WIN32
        file = _wfopen(ifile_names[i].wstring(), L"rb");
#else
        file = fopen(ifile_names[i].c_str(), "rb");
#endif
        if (file != nullptr) {
            int N = 0;
            fread(&N, sizeof(int), 1, file);
            if (N > 0) {
                Ns.push_back(N);
            } else {
                throw std::runtime_error("invalid content in file: " + ifile_names[i].string());
            }
        } else {
            throw std::runtime_error("file: " + ifile_names[i].string() + "was not open");
        }
    }
}
bool DataStorage::readw_txt()
{
    // проверка не выходит ли буфер за границы
    if (current_cursor < ifile_names.size()) {
        offsets.clear();
        if (current_cursor + ibuff_size >= ifile_names.size()) {  // если файлов осталось меньше чем размер buff_size
            ibuff_size = ifile_names.size() - current_cursor;
            ifiles.clear();
            ifiles.assign(ibuff_size, nullptr);
        }
        offsets.reserve(ibuff_size);
        size_t index_capacity = 0;
        for (size_t i = 0; i < ibuff_size; ++i) {
            index_capacity += Ns[current_cursor + i];
            offsets.push_back(index_capacity);
        }

        //  чтение файлов
        if (ibuff_size >= 8) {
            // выделение памяти для массивов
            for (size_t i = 0; i < column_list.size(); ++i) {
                std::visit(ResizeVisitor{offsets.back()}, column_list[i]);
            }
            read_parallel_txt();
            current_cursor += ibuff_size;
        } else {
            // выделение памяти для массивов
            for (size_t i = 0; i < column_list.size(); ++i) {
                std::visit(ReservVisitor{offsets.back()}, column_list[i]);
            }
            read_consistent_txt();
            current_cursor += ibuff_size;
        }
        return true;
    }
    return false;
}
bool DataStorage::readw_bin()
{
    if (current_cursor < ifile_names.size()) {
        offsets.clear();
        if (current_cursor + ibuff_size >= ifile_names.size()) {  // если файлов осталось меньше чем размер buff_size
            ibuff_size = ifile_names.size() - current_cursor;
            ifiles.assign(ibuff_size, nullptr);
        }
        size_t index_capacity = 0;
        offsets.reserve(ibuff_size);
        for (size_t i = 0; i < ibuff_size; ++i) {
            index_capacity += Ns[current_cursor + i];
            offsets.push_back(index_capacity);
        }
        // резервирование памяти для столбцов
        if (ibuff_size >= 8) {
            for (size_t i = 0; i < column_list.size(); ++i) {
                std::visit(ResizeVisitor{offsets.back()}, column_list[i]);
            }
            read_parallel_bin();
        } else {
            for (size_t i = 0; i < column_list.size(); ++i) {
                std::visit(ResizeVisitor{offsets.back()}, column_list[i]);
            }
            read_consistent_bin();
        }
        current_cursor += ibuff_size;
        return true;
    } else {
        return false;
    }
}
void DataStorage::read_consistent_bin()
{
    for (size_t i = 0; i < ibuff_size; ++i) {
#ifdef _WIN32
        ifiles[i] = _wfopen(ifile_names[current_cursor + i].wstring().c_str(), "rb");
#else
        ifiles[i] = fopen(ifile_names[current_cursor + i].c_str(), "rb");
#endif
        if (!ifiles[i]) {
            Ns[current_cursor + i] = 0;
            continue;
            // TODO: добавить логи
        }
        std::vector<char> buffer(ifile_buffer_size);
        setvbuf(ifiles[i], buffer.data(), _IOFBF, buffer.size());
        int _N;
        fread(&_N, sizeof(int), 1, ifiles[i]);
        for (int j = 0; j < Ns[current_cursor + i]; ++j) {
            for (size_t col = 0; col < column_list.size(); ++col) {
                size_t index = offsets[i] - offsets[0] + j;
                if (index >= offsets.back()) {
                    // TODO: добавить логи
                    break;
                }
                auto ptr = std::visit(GetPointerVisitor{index}, column_list[col]);
                size_t size = std::visit(GetTypeSizeVisitor{}, column_list[col]);
                if (ptr != nullptr && size != 0) {
                    if (fread(ptr, size, 1, ifiles[i]) != 1) {
                        break;
                    }
                }
            }
        }
        fclose(ifiles[i]);
    }
}
void DataStorage::read_consistent_txt()
{
    for (size_t i = 0; i < ibuff_size; ++i) {
#ifdef _WIN32
        ifiles[i] = _wfopen(ifile_names[current_cursor + i].wstring().c_str(), "r");
#else
        ifiles[i] = fopen(ifile_names[current_cursor + i].c_str(), "r");
#endif
        if (!ifiles[i]) {
            Ns[current_cursor + i] = 0;
            continue;
            // TODO: добавить логи (ошибка открытия файла)
        }
        std::vector<char> buffer(ifile_buffer_size);  // создание буфера для чтения из файла
        setvbuf(ifiles[i], buffer.data(), _IOFBF, buffer.size());
        fseek(ifiles[i], 0, SEEK_END);

        std::streamsize size = ftell(ifiles[i]);
        fseek(ifiles[i], 0, SEEK_SET);  // вычисляем размер файла
        std::string content(size, '\0');

        if (fread(content.data(), 1, size, ifiles[i]) != static_cast<size_t>(size)) {
            // TODO: добавить логи
            Ns[i] = 0;
            continue;
        }
        fclose(ifiles[i]);
        // Парсинг файла
        int _N;                                                       // пропускаем число строк
        const char* content_ptr = content.c_str();                    // указатель на начало content
        const char* content_end = content_ptr + content.size();       // указатель на конец content
        auto result = std::from_chars(content_ptr, content_end, _N);  // парсинг числового значения из
                                                                      // content
        // если данные не были распаршены
        if (result.ec != std::errc() || _N != Ns[current_cursor + i]) {
            // TODO: добавить логи
            Ns[current_cursor + i] = 0;
            continue;
        }
        content_ptr = result.ptr;
        while (content_ptr < content_end && (*content_ptr == '\n' || *content_ptr == '\r')) ++content_ptr;
        int n = 0;
        while (content_ptr < content_end && n < Ns[current_cursor + i]) {
            for (size_t col = 0; col < column_list.size(); ++col) {
                while (content_ptr < content_end && (*content_ptr == ' ' || *content_ptr == '\t')) content_ptr++;
                double value;
                auto res = std::from_chars(content_ptr, content_end, value);

                if (res.ec != std::errc()) {
                    // TODO: добавить логи
                } else {
                    std::visit(PushValueVisitor{value}, column_list[col]);
                    content_ptr = res.ptr;
                }
            }
            while (content_ptr < content_end && (*content_ptr == '\n' || *content_ptr == '\r')) ++content_ptr;
            n++;
        }
    }
}
void DataStorage::read_parallel_bin()
{
#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < ibuff_size; ++i) {
#ifdef _WIN32
        ifiles[i] = _wfopen(ifile_names[current_cursor + i].wstring().c_str(), "rb");
#else
        ifiles[i] = fopen(ifile_names[current_cursor + i].c_str(), "rb");
#endif
        if (!ifiles[i]) {
            Ns[current_cursor + i] = 0;
            continue;
            // TODO: добавить логи
        }
        std::vector<char> buffer(ifile_buffer_size);
        setvbuf(ifiles[i], buffer.data(), _IOFBF, buffer.size());
        int _N;
        fread(&_N, sizeof(int), 1, ifiles[i]);
        for (int j = 0; j < Ns[current_cursor + i]; ++j) {
            for (size_t col = 0; col < column_list.size(); ++col) {
                size_t index = offsets[i] - offsets[0] + j;
                if (index >= offsets.back()) {
                    // TODO: добавить логи
                    break;
                }
                auto ptr = std::visit(GetPointerVisitor{index}, column_list[col]);
                size_t size = std::visit(GetTypeSizeVisitor{}, column_list[col]);
                if (ptr != nullptr && size != 0) {
                    if (fread(ptr, size, 1, ifiles[i]) != 1) {
                        break;
                    }
                }
            }
        }
        fclose(ifiles[i]);
    }
}
void DataStorage::read_parallel_txt()
{
#pragma omp parallel for schedule(dinamic)
    for (size_t i = 0; i < ibuff_size; ++i) {
#ifdef _WIN32
        ifiles[i] = _wfopen(ifile_names[current_cursor + i].wstring().c_str(), "r");
#else
        ifiles[i] = fopen(ifile_names[current_cursor + i].c_str(), "r");
#endif
        if (!ifiles[i]) {
            // TODO: добавить логи (ошибка открытия файла)
            Ns[current_cursor + i] = 0;
            continue;
        }
        std::vector<char> buffer(ifile_buffer_size);  // создание буфера для чтения из файла
        setvbuf(ifiles[i], buffer.data(), _IOFBF, buffer.size());

        fseek(ifiles[i], 0, SEEK_END);
        std::streamsize size = ftell(ifiles[i]);
        fseek(ifiles[i], 0, SEEK_SET);  // вычисляем размер файла
        std::string content(size, '\0');

        if (fread(content.data(), 1, size, ifiles[i]) != static_cast<size_t>(size)) {
            // TODO: добавить логи
            Ns[current_cursor + i] = 0;
            continue;
        }
        fclose(ifiles[i]);
        // Парсинг файла
        int _N;                                                       // пропускаем число строк
        const char* content_ptr = content.c_str();                    // указатель на начало content
        const char* content_end = content_ptr + content.size();       // указатель на конец content
        auto result = std::from_chars(content_ptr, content_end, _N);  // парсинг числового значения из
                                                                      // content
        // если данные не были распаршены
        if (result.ec != std::errc() || _N != Ns[current_cursor + i]) {
            // TODO: добавить логи
            Ns[current_cursor + i] = 0;
            continue;
        }
        // пропускаем \n
        content_ptr = result.ptr;
        while (content_ptr < content_end && (*content_ptr == '\n' || *content_ptr == '\r')) ++content_ptr;
        int n = 0;
        while (content_ptr < content_end && n < Ns[current_cursor + i]) {
            for (size_t col = 0; col < column_list.size(); ++col) {
                while (content_ptr < content_end && (*content_ptr == ' ' || *content_ptr == '\t')) content_ptr++;
                // TODO: добавить логи
                double value;
                auto res = std::from_chars(content_ptr, content_end, value);
                if (res.ec != std::errc()) {
                    // TODO: добавить логи
                    break;
                } else {
                    std::visit(WriteValueVisitor{value, offsets[i] - offsets[0] + n}, column_list[col]);
                    content_ptr = res.ptr;
                }
                if (content_ptr < content_end && *content_ptr == ',') ++content_ptr;
            }
            while (content_ptr < content_end && (*content_ptr == '\n' || *content_ptr == '\r')) ++content_ptr;
            n++;
        }
    }
}