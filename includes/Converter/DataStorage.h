#pragma once
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <variant>
#include <vector>

class DataStorage
{
   private:
    enum Type { Int, Double, NullPtr };
    std::vector<FILE*> ifiles;                       // потоки входных файлов (данных)
    std::vector<std::filesystem::path> ifile_names;  // пути к файлам
    // файлы должны имет одинаковую структуру иначе будет ошибка исполнения
    std::vector<std::variant<std::vector<double>*, std::vector<int>*, std::nullptr_t>> column_list;  // лист колонок
                                                                                                     // (структура файлов)
    //---------список всех возможных колонок (не все могут использоваться одновременно)---------
    std::vector<double> x, y, z;     // координаты
    std::vector<double> vx, vy, vz;  // скорости
    std::vector<double> rho, e, m;   // плотность, внутреняя энергия,масса
    std::vector<int> ind_sph;
    std::vector<double> t_MCYS;
    std::vector<int> Ns;       // число строк в каждом файле
    std::vector<double> t;     // время в модели(записано в файлах вторым числом)
    size_t ibuff_size;         // число читаемых за раз файлов
    size_t current_cursor;     // номер читаемого в данный момент файла
    size_t ifile_buffer_size;  // размер буфера для чтения одного файла
    // size_t capacity; - не используется
    std::vector<size_t> offsets;  // вектор с смещениями
    void read_parallel_txt();
    void read_consistent_txt();
    void read_parallel_bin();
    void read_consistent_bin();

   public:
    DataStorage(const std::vector<std::filesystem::path>& pathes);
    DataStorage(const DataStorage&) = delete;             // запрет конструктора копирования
    DataStorage& operator=(const DataStorage&) = delete;  // запрет копирования присваиванием
    std::vector<double>& get_x();
    std::vector<double>& get_y();
    std::vector<double>& get_z();
    std::vector<double>& get_vx();
    std::vector<double>& get_vy();
    std::vector<double>& get_vz();
    std::vector<double>& get_rho();
    std::vector<double>& get_e();
    std::vector<double>& get_m();
    std::vector<int>& get_ind_sph();
    std::vector<double>& get_t_MCYS();
    std::vector<int>& get_Ns();
    std::vector<size_t>& get_offsets();
    std::vector<double>& get_t();
    std::vector<std::filesystem::path> get_last_file_names();
    size_t get_ibuff_size();
    void set_buff_size(const size_t size);
    void setup_columns(const std::vector<std::string>& columns);
    bool readw_txt();                      // чтение txt с перезаписью в колонки
    bool readw_bin();                      // чтение bin с перезаписью в колонки
    void reset_cursor() noexcept;          // сброс курсора
    void set_cursor(const size_t cursor);  // установка положения курсора
    void load_file_metadate_txt();         // загрузка метаданных файлов(число строк в каждом файле) и резервирование памяти
    void load_file_metadate_bin();         // загрузка метаданных файлов(число строк в каждом файле) и резервирование памяти
    void set_file_buffer_size(const size_t size);
};