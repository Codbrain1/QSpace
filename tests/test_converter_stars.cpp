#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "ConstantsParametrs.h"
#include "Converter/AgregateStructures.h"
#include "Converter/Converter_Stars.h"
#include "Converter/DataStorage.h"

namespace STARS_TEST
{
const std::filesystem::path TEMP_OUTPUT_DIR =
    "/home/mask/Desktop/nir5semester/TxtToGrdConverter/tests/test_output";  // выходная директория для сконвертированных
                                                                            // файлов
const std::filesystem::path REFERENCE_ROOT =
    "/home/mask/Desktop/nir5semester/TxtToGrdConverter/tests/Datasets/RightData";  // директория с файлами для проверки
                                                                                   // конвертации
const std::filesystem::path INITIAL_DATA_PATH =
    "/home/mask/Desktop/nir5semester/TxtToGrdConverter/tests/Datasets/InitialData/";

std::vector<std::filesystem::path> pathes;  // пути к файлам конвертации
ParametrsList::iniConstants c;              // константы для конвертации
count_cell Nbxy;                            // число точек в выходном grd файле
lim<double> x, y, z;                        // отображаемая область
constexpr float FLOAT_EPS = 1e-4f;          // прогрешность в рамках сравнения файлов
}  // namespace STARS_TEST

// ---------------------------------------------------------------------
// Утилиты
// ---------------------------------------------------------------------
void setup_output_dir()
{
    using namespace STARS_TEST;
    if (std::filesystem::exists(TEMP_OUTPUT_DIR)) std::filesystem::remove_all(TEMP_OUTPUT_DIR);
    std::filesystem::create_directories(TEMP_OUTPUT_DIR);
}
// ---------------------------------------------------------------------
// Инициализация параметров
// ---------------------------------------------------------------------
void INIT_CONVERTER_PARAMS(std::string_view files_type)
{
    using namespace STARS_TEST;

    x = {4, -4};
    y = {4, -4};
    z = {4, -4};  // область отображения

    c.gamma = 1.666667;  // макроконстанты
    c.hb = 0.01;         //
    c.Km = 3.72;         //
    c.Kr = 0.9;          //

    Nbxy.Nx = static_cast<int>((x.max - x.min + c.hb) / c.hb + 0.5);  // число точек в grd файле по x
    Nbxy.Ny = static_cast<int>((y.max - y.min + c.hb) / c.hb + 0.5);  // число точек в grd файле по y

    if (files_type == ParametrsList::is_bin_ifiles) {
        for (const auto& entry : std::filesystem::directory_iterator(INITIAL_DATA_PATH / "bin/STARS")) {
            // Пропускаем подкаталоги, если нужны только файлы
            if (entry.is_regular_file()) {
                // entry.path() – полный путь (можно .string(), .u8string() и т.д.)
                pathes.emplace_back(entry.path());
            }
        }
    } else if (files_type == ParametrsList::is_txt_ifiles) {
        for (const auto& entry : std::filesystem::directory_iterator(INITIAL_DATA_PATH / "txt/STARS")) {
            // Пропускаем подкаталоги, если нужны только файлы
            if (entry.is_regular_file()) {
                // entry.path() – полный путь (можно .string(), .u8string() и т.д.)
                pathes.emplace_back(entry.path());
            }
        }
    } else {
        throw std::runtime_error("указан неверный тип файла");
    }
}
void INIT_DATASTORAGE(DataStorage& datastorage)
{
    // задание буфера чтения для файлов
    datastorage.set_buff_size(8);
    // установка колоноко для темной материи
    datastorage.setup_columns({std::string(ParametrsList::X), std::string(ParametrsList::Y), std::string(ParametrsList::Z),
                               std::string(ParametrsList::VX), std::string(ParametrsList::VY),
                               std::string(ParametrsList::VZ), std::string(ParametrsList::m)});
}
// чтение данных их файлов (из текстовых или бинарных файлов)
void BINorTXT_READ(std::string_view ifile_type, DataStorage& datastorage)
{
    if (ifile_type == ParametrsList::is_bin_ifiles) {
        datastorage.load_file_metadate_bin();
        datastorage.readw_bin();
    } else if (ifile_type == ParametrsList::is_txt_ifiles) {
        datastorage.load_file_metadate_txt();
        datastorage.readw_txt();
    } else {
        throw std::invalid_argument("неизвестный тип считываемого файла");
    }
}
void CONVERT(DataStorage& datastorage, std::string_view ofile_type, std::vector<std::string> Z_col_list,
             std::pair<std::string, std::string> projection, std::filesystem::path proj_path)
{
    std::filesystem::create_directories(STARS_TEST::TEMP_OUTPUT_DIR / proj_path);
    Converter_Stars cs(datastorage, STARS_TEST::c, STARS_TEST::Nbxy, STARS_TEST::TEMP_OUTPUT_DIR / proj_path);
    cs.set_limits(STARS_TEST::x, STARS_TEST::y, STARS_TEST::z);
    cs.setup_output_data(Z_col_list, projection);
    cs.convert();
    if (ofile_type == ParametrsList::is_bin_grd)  // запись (в бинарный файл)
    {
        cs.save_grd_bin();

    } else if (ofile_type == ParametrsList::is_txt_grd)  // запись (в текстовый файл)
    {
        cs.save_grd_txt();
    }
}
// настройка начального состояния конвертации и ее запуск
void START_CONVERT(std::string_view ifile_type, std::string_view ofile_type)
{
    INIT_CONVERTER_PARAMS(ifile_type);
    DataStorage datastorage(STARS_TEST::pathes);
    INIT_DATASTORAGE(datastorage);
    BINorTXT_READ(ifile_type, datastorage);
    std::vector<std::string> Z_out_par;
    // for (auto i : ParametrsList::Z_outParams) Z_out_par.push_back(std::string(i));
    Z_out_par.push_back(std::string(ParametrsList::Z_outParams_Rho));
    Z_out_par.push_back(std::string(ParametrsList::Z_outParams_LgRho));
    Z_out_par.push_back(std::string(ParametrsList::Z_outParams_Sigma));
    Z_out_par.push_back(std::string(ParametrsList::Z_outParams_LgSigma));
    Z_out_par.push_back(std::string(ParametrsList::Z_outParams_Vx));
    Z_out_par.push_back(std::string(ParametrsList::Z_outParams_Vy));
    Z_out_par.push_back(std::string(ParametrsList::Z_outParams_Vz));
    // Z_out_par.push_back(std::string(ParametrsList::Z_outParams_Vfi));
    // Z_out_par.push_back(std::string(ParametrsList::Z_outParams_Vr));
    Z_out_par.push_back(std::string(ParametrsList::Z_outParams_LgRho));

    CONVERT(datastorage, ofile_type, Z_out_par, {std::string(ParametrsList::X), std::string(ParametrsList::Y)}, "XY");
    CONVERT(datastorage, ofile_type, Z_out_par, {std::string(ParametrsList::X), std::string(ParametrsList::Z)}, "XZ");
    CONVERT(datastorage, ofile_type, Z_out_par, {std::string(ParametrsList::Y), std::string(ParametrsList::Z)}, "YZ");
}
// ---------------------------------------------------------------------
// Сравнение Surfer 6
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// Сравнение Surfer 6 — точное, надёжное, без "магических" чисел
// ---------------------------------------------------------------------
bool compare_surfer6_bin(const std::filesystem::path& gen, const std::filesystem::path& ref)
{
    std::ifstream fgen(gen, std::ios::binary), fref(ref, std::ios::binary);
    if (!fgen || !fref) return false;

    // --- Эталон: читаем header_size (гарантированно валиден) ---
    std::int16_t header_size_ref;
    fref.seekg(4);  // пропускаем "DSRB"
    fref.read(reinterpret_cast<char*>(&header_size_ref), 2);

    // --- Генерируемый файл: проверяем DSRB и header_size ---
    char id_gen[4];
    std::int16_t header_size_gen;
    fgen.read(id_gen, 4);
    if (std::string(id_gen, 4) != "DSBB") {
        std::cout << "Генерируемый файл: неверная сигнатура DSBB\n";
        return false;
    }
    fgen.read(reinterpret_cast<char*>(&header_size_gen), 2);
    if (header_size_gen != header_size_ref) {
        std::cout << "Размер заголовка не совпадает: " << header_size_gen << " vs " << header_size_ref << "\n";
        return false;
    }

    // --- Переход к данным ---
    fgen.seekg(header_size_gen, std::ios::beg);
    fref.seekg(header_size_ref, std::ios::beg);

    // --- Сравнение float-ов с учётом NODATA ---
    float vgen, vref;
    const float NODATA = 1.70141e+38f;
    const float EPS = STARS_TEST::FLOAT_EPS;

    while (true) {
        bool read_gen = static_cast<bool>(fgen.read(reinterpret_cast<char*>(&vgen), 4));
        bool read_ref = static_cast<bool>(fref.read(reinterpret_cast<char*>(&vref), 4));

        // Если один поток закончился, а другой — нет
        if (read_gen != read_ref) {
            std::cout << "Разная длина данных\n";
            return false;
        }

        // Если оба закончились — успех
        if (!read_gen && !read_ref) {
            return true;
        }

        // Сравниваем значения
        if (std::abs(vgen - vref) > EPS) {
            if (std::abs(vgen - NODATA) > 1e-10f || std::abs(vref - NODATA) > 1e-10f) {
                std::cout << "ПРОГРАММА: " << vgen << "\nЭТАЛОН: " << vref << std::endl;
                return false;
            }
        }
    }
}
// ---------------------------------------------------------------------
// Сравнение Surfer 6 — текстовый формат (.grd)
// ---------------------------------------------------------------------
bool compare_surfer6_txt(const std::filesystem::path& gen, const std::filesystem::path& ref)
{
    std::ifstream fgen(gen), fref(ref);
    if (!fgen || !fref) return false;

    std::string line_gen, line_ref;

    while (std::getline(fgen, line_gen) && std::getline(fref, line_ref)) {
        // Пропускаем пустые строки
        if (line_gen.empty() && line_ref.empty()) continue;

        // Сравниваем строки посимвольно
        if (line_gen != line_ref) {
            // Попытка разбора как числа с плавающей точкой
            std::istringstream sg(line_gen), sr(line_ref);
            std::string token_gen, token_ref;

            bool mismatch = false;
            while (sg >> token_gen, sr >> token_ref) {
                try {
                    float vgen = std::stof(token_gen);
                    float vref = std::stof(token_ref);

                    const float NODATA = 1.70141e+38f;
                    const float EPS = STARS_TEST::FLOAT_EPS;

                    if (std::abs(vgen - vref) > EPS) {
                        if (std::abs(vgen - NODATA) > 1e-10f || std::abs(vref - NODATA) > 1e-10f) {
                            std::cout << "ПРОГРАММА: " << vgen << "\nЭТАЛОН: " << vref << std::endl;
                            mismatch = true;
                        }
                    }
                } catch (...) {
                    // Не число — сравниваем как строку
                    if (token_gen != token_ref) {
                        mismatch = true;
                    }
                }
            }

            // Если один поток дал больше токенов
            if (sg >> token_gen || sr >> token_ref) {
                mismatch = true;
            }

            if (mismatch) {
                std::cout << "НЕ СОВПАДАЕТ СТРОКА:\n"
                          << "gen: " << line_gen << "\n"
                          << "ref: " << line_ref << std::endl;
                return false;
            }
        }
    }

    // Проверяем, что оба файла закончились одновременно
    bool end_gen = fgen.eof() && !fgen.fail();
    bool end_ref = fref.eof() && !fref.fail();

    if (end_gen != end_ref) {
        std::cout << "Разная длина текстовых файлов\n";
        return false;
    }

    return true;
}
// ---------------------------------------------------------------------
// Парсинг имени: DM_XY_Rho_t_2130.7.grd → DM, XY, Rho
// ---------------------------------------------------------------------
struct ParsedFilename {
    std::string type;        // DM
    std::string projection;  // XY
    std::string param;       // LgRho
    std::string time;        // 2130.694666
    std::string full_name;   // DM_XY_LgRho_2130.694666.grd (полное имя файла)
};
// ---------------------------------------------------------------------
// Парсинг: DM_XY_LgRho_2130.694666.grd
// ---------------------------------------------------------------------
ParsedFilename parse_filename(const std::string& filename)
{
    std::filesystem::path path(filename);
    std::string stem = path.stem().string();      // DM_XY_LgRho_2130.694666
    std::string ext = path.extension().string();  // .grd

    std::vector<std::string> parts;
    std::istringstream ss(stem);
    std::string token;
    while (std::getline(ss, token, '_')) {
        parts.push_back(token);
    }

    if (parts.size() < 4) {
        throw std::runtime_error("Неправильное имя файла (ожидается минимум 4 части): " + filename);
    }
    if (parts[0] != "STARS") {
        throw std::runtime_error("Ожидался префикс STARS: " + filename);
    }

    ParsedFilename p;
    p.type = parts[0];        // DM
    p.projection = parts[1];  // XY
    p.param = parts[2];       // LgRho

    // time — всё, что после param
    p.time = parts[3];
    for (size_t i = 4; i < parts.size(); ++i) {
        p.time += "_" + parts[i];
    }

    // Полное имя файла (с расширением)
    p.full_name = stem + ext;

    return p;
}
// ---------------------------------------------------------------------
// Поиск эталона: тот же файл, но в RightData/bin/DM/XY/LgRho/
// ---------------------------------------------------------------------
std::filesystem::path find_reference(const std::filesystem::path& generated, std::string_view output_type)
{
    using namespace STARS_TEST;
    std::string filename = generated.filename().string();
    ParsedFilename p = parse_filename(filename);

    // Определяем поддиректорию: bin или txt
    std::string ext_dir = (output_type == ParametrsList::is_bin_grd) ? "bin" : "txt";

    // Путь к эталону: REFERENCE_ROOT / bin|txt / DM / XY / LgRho / <полное_имя_файла>
    return REFERENCE_ROOT / ext_dir / "STARS" / p.projection / p.param / p.full_name;
}

// ---------------------------------------------------------------------
// Основной тест
// ---------------------------------------------------------------------
void RUN_DM_TEST()
{
    using namespace STARS_TEST;

    setup_output_dir();  // задаем и очищаем директорию с временными файлами

    struct TestCase {  // тест и его тип
        std::string_view input_type;
        std::string_view output_type;
        std::string_view name;
    };

    const std::vector<TestCase> tests = {
        {ParametrsList::is_bin_ifiles, ParametrsList::is_bin_grd, "Binary to Binary GRD"},
        // {ParametrsList::is_txt_ifiles, ParametrsList::is_bin_grd, "Text to Binary GRD"},
    };

    for (const auto& tc : tests) {
        std::cout << "\n=== ТЕСТ: " << tc.name << " ===\n";

        pathes.clear();
        START_CONVERT(tc.input_type, tc.output_type);

        std::vector<std::filesystem::path> generated;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(TEMP_OUTPUT_DIR)) {
            if (entry.is_regular_file()) {
                generated.push_back(entry.path());
            }
        }

        if (generated.empty()) throw std::runtime_error("Нет сгенерированных файлов: " + std::string(tc.name));

        for (const auto& gen : generated) {
            std::filesystem::path ref = find_reference(gen, tc.output_type);

            if (!std::filesystem::exists(ref)) throw std::runtime_error("ЭТАЛОН НЕ НАЙДЕН: " + ref.string());

            bool ok = (tc.output_type == ParametrsList::is_bin_grd) ? compare_surfer6_bin(gen, ref)
                                                                    : compare_surfer6_txt(gen, ref);

            if (!ok)
                throw std::runtime_error("НЕ СОВПАДАЕТ: " + gen.filename().string() + "\n  gen: " + gen.string() +
                                         "\n  ref: " + ref.string());

            std::cout << "  " << gen.lexically_relative(TEMP_OUTPUT_DIR) << " -> [PASS]\n";
        }

        std::cout << "  [OK] Тест '" << tc.name << "' пройден.\n";
    }

    std::cout << "\nВСЕ ТЕСТЫ ПРОЙДЕНЫ!\n";
}
// ---------------------------------------------------------------------
// main
// ---------------------------------------------------------------------
int main()
{
    try {
        RUN_DM_TEST();
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "TEST FAILED: " << ex.what() << std::endl;
        return 1;
    }
}