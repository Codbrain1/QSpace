#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "ConstantsParametrs.h"
#include "Converter/DataStorage.h"
namespace test_file
{
std::filesystem::path test_path("/home/mask/Desktop/nir5semester/TxtToGrdConverter/tests/Datasets/");
const size_t NUM_FILES = 10;  // Генерируем 10 файлов (≥8) для теста параллельной обработки
const int N = 5;              // Количество строк в каждом файле
const double t = 0.5;         // время (внутри модели) в каждом файле
// Структура для хранения сгенерированных данных
struct GeneratedData {
    std::vector<std::vector<double>> columns;  // 9 столбцов: x, y, z, rho, vx, vy, vz, e, m
    std::vector<int> ind_sph;
};

// RAII-объект для удаления файлов
struct FileCleaner {
    std::vector<std::filesystem::path> files;
    ~FileCleaner()
    {
        for (const auto& file : files) {
            std::filesystem::remove(file);
        }
    }
};

// Генерация случайных данных
GeneratedData generate_random_data(size_t rows)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1000.0);
    std::uniform_int_distribution<int> ind_dist(900, 1000);

    GeneratedData data;
    data.columns.resize(rows, std::vector<double>(9));
    data.ind_sph.resize(rows);

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < 9; ++j) {
            data.columns[i][j] = dist(gen);
        }
        data.ind_sph[i] = ind_dist(gen);
    }
    return data;
}

// Генерация текстового файла в научном формате
void generate_txt_file(const std::filesystem::path& path, const GeneratedData& data, int n, double _t)
{
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Не удалось создать текстовый файл: " + path.string());
    }
    out << std::scientific << std::setprecision(15);  // Научный формат с высокой точностью
    out << n << " ";
    out << _t << "\n";
    for (size_t i = 0; i < data.columns.size(); ++i) {
        for (double val : data.columns[i]) {
            out << val << " ";
        }
        out << data.ind_sph[i] << "\n";
    }
    out.close();
}

// Генерация бинарного файла
void generate_bin_file(const std::filesystem::path& path, const GeneratedData& data, int n, double _t)
{
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Не удалось создать бинарный файл: " + path.string());
    }
    out.write(reinterpret_cast<const char*>(&n), sizeof(int));
    out.write(reinterpret_cast<const char*>(&_t), sizeof(double));
    for (size_t i = 0; i < data.columns.size(); ++i) {
        for (double val : data.columns[i]) {
            out.write(reinterpret_cast<const char*>(&val), sizeof(double));
        }
        out.write(reinterpret_cast<const char*>(&data.ind_sph[i]), sizeof(int));
    }
    out.close();
}

// Генерация тестовых файлов и сохранение данных
std::pair<std::vector<std::filesystem::path>, std::vector<GeneratedData>> generate_test_files(bool binary = false)
{
    std::vector<std::filesystem::path> files;
    std::vector<GeneratedData> generated_data;

    for (size_t i = 0; i < NUM_FILES; ++i) {
        std::string filename = binary ? "test_bin_" + std::to_string(i) + ".bin" : "test_txt_" + std::to_string(i) + ".txt";
        std::filesystem::path file_path = test_path / filename;
        auto data = generate_random_data(N);
        if (binary) {
            generate_bin_file(file_path, data, N, t);
        } else {
            generate_txt_file(file_path, data, N, t);
        }
        files.push_back(file_path);
        generated_data.push_back(data);
    }
    return {files, generated_data};
}
}  // namespace test_file

void ReadTXT_Test()
{
    using namespace test_file;
    FileCleaner cleaner;
    auto [files, generated_data] = generate_test_files(false);  // Генерируем текстовые файлы
    cleaner.files = files;                                      // Передаём файлы в FileCleaner для удаления после теста

    DataStorage storage(files);
    storage.setup_columns({std::string(ParametrsList::X), std::string(ParametrsList::Y), std::string(ParametrsList::Z),
                           std::string(ParametrsList::Rho), std::string(ParametrsList::VX), std::string(ParametrsList::VY),
                           std::string(ParametrsList::VZ), std::string(ParametrsList::e), std::string(ParametrsList::m),
                           std::string(ParametrsList::ind_sph)});
    storage.load_file_metadate_txt();

    // Счётчик для отслеживания текущего файла
    size_t current_file_idx = 0;
    while (storage.readw_txt()) {
        size_t ibuff_size = storage.get_ibuff_size();  // Количество файлов в текущем окне
        for (size_t i = 0; i < ibuff_size; ++i) {
            if (current_file_idx + i >= files.size()) {
                throw std::runtime_error("Превышено количество файлов: " + std::to_string(current_file_idx + i));
            }
        }

        // Проверяем данные для текущего окна
        size_t offset = 0;
        for (size_t file_idx = current_file_idx; file_idx < current_file_idx + ibuff_size && file_idx < files.size();
             ++file_idx) {
            const auto& data = generated_data[file_idx];
            if (storage.get_t()[file_idx] != t) {
                throw std::runtime_error("ошибка чтения времени в файле " + files[file_idx].string());
            }
            for (size_t i = 0; i < N; ++i) {
                std::stringstream ss;
                ss << std::scientific << std::setprecision(15);
                if (std::abs(data.columns[i][0] - storage.get_x()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения x в файле " << files[file_idx].string() << ": " << data.columns[i][0]
                       << " != " << storage.get_x()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][0] - storage.get_x()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][1] - storage.get_y()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения y в файле " << files[file_idx].string() << ": " << data.columns[i][1]
                       << " != " << storage.get_y()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][1] - storage.get_y()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][2] - storage.get_z()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения z в файле " << files[file_idx].string() << ": " << data.columns[i][2]
                       << " != " << storage.get_z()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][2] - storage.get_z()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][3] - storage.get_rho()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения rho в файле " << files[file_idx].string() << ": " << data.columns[i][3]
                       << " != " << storage.get_rho()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][3] - storage.get_rho()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][4] - storage.get_vx()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения vx в файле " << files[file_idx].string() << ": " << data.columns[i][4]
                       << " != " << storage.get_vx()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][4] - storage.get_vx()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][5] - storage.get_vy()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения vy в файле " << files[file_idx].string() << ": " << data.columns[i][5]
                       << " != " << storage.get_vy()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][5] - storage.get_vy()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][6] - storage.get_vz()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения vz в файле " << files[file_idx].string() << ": " << data.columns[i][6]
                       << " != " << storage.get_vz()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][6] - storage.get_vz()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][7] - storage.get_e()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения e в файле " << files[file_idx].string() << ": " << data.columns[i][7]
                       << " != " << storage.get_e()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][7] - storage.get_e()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][8] - storage.get_m()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения m в файле " << files[file_idx].string() << ": " << data.columns[i][8]
                       << " != " << storage.get_m()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][8] - storage.get_m()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (data.ind_sph[i] != storage.get_ind_sph()[offset + i]) {
                    ss << "Ошибка чтения ind_sph в файле " << files[file_idx].string() << ": " << data.ind_sph[i]
                       << " != " << storage.get_ind_sph()[offset + i];
                    throw std::runtime_error(ss.str());
                }
            }
            offset += N;  // Смещение для следующего файла в окне
        }
        current_file_idx += ibuff_size;  // Переходим к следующей порции файлов
    }

    // Проверяем, что все файлы были обработаны
    if (current_file_idx != files.size()) {
        throw std::runtime_error("Не все файлы были прочитаны: обработано " + std::to_string(current_file_idx) + " из " +
                                 std::to_string(files.size()));
    }
}

void ReadBIN_Test()
{
    using namespace test_file;
    FileCleaner cleaner;
    auto [files, generated_data] = generate_test_files(true);  // Генерируем бинарные файлы
    cleaner.files = files;                                     // Передаём файлы в FileCleaner для удаления после теста

    DataStorage storage(files);
    storage.setup_columns({std::string(ParametrsList::X), std::string(ParametrsList::Y), std::string(ParametrsList::Z),
                           std::string(ParametrsList::Rho), std::string(ParametrsList::VX), std::string(ParametrsList::VY),
                           std::string(ParametrsList::VZ), std::string(ParametrsList::e), std::string(ParametrsList::m),
                           std::string(ParametrsList::ind_sph)});
    storage.load_file_metadate_bin();

    // Счётчик для отслеживания текущего файла
    size_t current_file_idx = 0;
    while (storage.readw_bin()) {
        size_t ibuff_size = storage.get_ibuff_size();  // Количество файлов в текущем окне
        for (size_t i = 0; i < ibuff_size; ++i) {
            if (current_file_idx + i >= files.size()) {
                throw std::runtime_error("Превышено количество файлов: " + std::to_string(current_file_idx + i));
            }
        }

        // Проверяем данные для текущего окна
        size_t offset = 0;
        for (size_t file_idx = current_file_idx; file_idx < current_file_idx + ibuff_size && file_idx < files.size();
             ++file_idx) {
            const auto& data = generated_data[file_idx];
            if (storage.get_t()[file_idx] != t) {
                throw std::runtime_error("ошибка чтения времени в файле " + files[file_idx].string());
            }
            for (size_t i = 0; i < N; ++i) {
                std::stringstream ss;
                ss << std::scientific << std::setprecision(15);
                if (std::abs(data.columns[i][0] - storage.get_x()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения x в файле " << files[file_idx].string() << ": " << data.columns[i][0]
                       << " != " << storage.get_x()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][0] - storage.get_x()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][1] - storage.get_y()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения y в файле " << files[file_idx].string() << ": " << data.columns[i][1]
                       << " != " << storage.get_y()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][1] - storage.get_y()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][2] - storage.get_z()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения z в файле " << files[file_idx].string() << ": " << data.columns[i][2]
                       << " != " << storage.get_z()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][2] - storage.get_z()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][3] - storage.get_rho()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения rho в файле " << files[file_idx].string() << ": " << data.columns[i][3]
                       << " != " << storage.get_rho()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][3] - storage.get_rho()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][4] - storage.get_vx()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения vx в файле " << files[file_idx].string() << ": " << data.columns[i][4]
                       << " != " << storage.get_vx()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][4] - storage.get_vx()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][5] - storage.get_vy()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения vy в файле " << files[file_idx].string() << ": " << data.columns[i][5]
                       << " != " << storage.get_vy()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][5] - storage.get_vy()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][6] - storage.get_vz()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения vz в файле " << files[file_idx].string() << ": " << data.columns[i][6]
                       << " != " << storage.get_vz()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][6] - storage.get_vz()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][7] - storage.get_e()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения e в файле " << files[file_idx].string() << ": " << data.columns[i][7]
                       << " != " << storage.get_e()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][7] - storage.get_e()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (std::abs(data.columns[i][8] - storage.get_m()[offset + i]) > 1e-6) {
                    ss << "Ошибка чтения m в файле " << files[file_idx].string() << ": " << data.columns[i][8]
                       << " != " << storage.get_m()[offset + i]
                       << " (разница: " << std::abs(data.columns[i][8] - storage.get_m()[offset + i]) << ")";
                    throw std::runtime_error(ss.str());
                }
                if (data.ind_sph[i] != storage.get_ind_sph()[offset + i]) {
                    ss << "Ошибка чтения ind_sph в файле " << files[file_idx].string() << ": " << data.ind_sph[i]
                       << " != " << storage.get_ind_sph()[offset + i];
                    throw std::runtime_error(ss.str());
                }
            }
            offset += N;  // Смещение для следующего файла в окне
        }
        current_file_idx += ibuff_size;  // Переходим к следующей порции файлов
    }

    // Проверяем, что все файлы были обработаны
    if (current_file_idx != files.size()) {
        throw std::runtime_error("Не все файлы были прочитаны: обработано " + std::to_string(current_file_idx) + " из " +
                                 std::to_string(files.size()));
    }
}

void loadmetadateTXTTest()
{
    using namespace test_file;
    FileCleaner cleaner;
    auto [files, generated_data] = generate_test_files(false);
    cleaner.files = files;  // Передаём файлы в FileCleaner для удаления после теста

    DataStorage storage(files);
    storage.setup_columns({std::string(ParametrsList::X), std::string(ParametrsList::Y), std::string(ParametrsList::Z),
                           std::string(ParametrsList::Rho), std::string(ParametrsList::VX), std::string(ParametrsList::VY),
                           std::string(ParametrsList::VZ), std::string(ParametrsList::e), std::string(ParametrsList::m),
                           std::string(ParametrsList::ind_sph)});
    storage.load_file_metadate_txt();
    for (size_t i = 0; i < storage.get_Ns().size(); ++i) {
        if (storage.get_Ns()[i] != N) {
            std::stringstream ss;
            ss << "Ошибка чтения метаданных текстового файла " << files[i].string() << ": " << N
               << " != " << storage.get_Ns()[i];
            throw std::runtime_error(ss.str());
        }
    }
}

void loadmetadateBINTest()
{
    using namespace test_file;
    FileCleaner cleaner;
    auto [files, generated_data] = generate_test_files(true);
    cleaner.files = files;  // Передаём файлы в FileCleaner для удаления после теста

    DataStorage storage(files);
    storage.setup_columns({std::string(ParametrsList::X), std::string(ParametrsList::Y), std::string(ParametrsList::Z),
                           std::string(ParametrsList::Rho), std::string(ParametrsList::VX), std::string(ParametrsList::VY),
                           std::string(ParametrsList::VZ), std::string(ParametrsList::e), std::string(ParametrsList::m),
                           std::string(ParametrsList::ind_sph)});
    storage.load_file_metadate_bin();
    for (size_t i = 0; i < storage.get_Ns().size(); ++i) {
        if (storage.get_Ns()[i] != N) {
            std::stringstream ss;
            ss << "Ошибка чтения метаданных бинарного файла " << files[i].string() << ": " << N
               << " != " << storage.get_Ns()[i];
            throw std::runtime_error(ss.str());
        }
    }
}

int main()
{
    try {
        std::cout << "Run ReadTXT_Test";
        ReadTXT_Test();
        std::cout << "Run ReadBIN_Test";
        ReadBIN_Test();
        std::cout << "Run loadmetadateTXTTest";
        loadmetadateTXTTest();
        std::cout << "Run loadmetadateBINTest";
        loadmetadateBINTest();

        std::cout << "Все тесты пройдены успешно!" << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}