#include "Converter/Converter_DarkMatter.h"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

void Converter_DarkMatter::convert()
{
    Z.reserve(Z_grd_list.size());
    auto [projection1, projection2, projection3] = get_projection();

    size_t files_size = data.get_last_file_names().size();
    ofiles_names.reserve(files_size * Z_grd_list.size());

    std::string xy = extract_upper_axis(std::string(XY.first)) + extract_upper_axis(std::string(XY.second));
    for (size_t file_type = 0; file_type < Z_grd_list.size(); ++file_type) {
        if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Rho) {
            //-----------------------Вычисление значений RHO--------------------------
            Z.push_back(std::vector<double>());
            calculate_Rho(projection1, projection2, projection3);
            create_output_directory("DM", xy, "Rho", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgRho) {
            //-----------------------Вычисление значений LG_RHO--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgRho(projection1, projection2, projection3);
            create_output_directory("DM", xy, "LgRho", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgSigma) {
            //-----------------------Вычисление значений LG_SIGMA--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgSigma(projection1, projection2, projection3);
            create_output_directory("DM", xy, "LgSigma", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Sigma) {
            //-----------------------Вычисление значений SIGMA--------------------------
            Z.push_back(std::vector<double>());
            calculate_Sigma(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("DM", xy, "Sigma", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vfi) {  // TODO: неправильное преобразование
            //-----------------------Вычисление значений Vfi--------------------------
            Z.push_back(std::vector<double>());
            calculate_Vfi(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("DM", xy, "Vfi", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vr) {
            //-----------------------Вычисление значений Vr--------------------------
            Z.push_back(std::vector<double>());
            calculate_Vr(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("DM", xy, "Vr", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vx) {
            //-----------------------Вычисление значений Vx--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, data.get_vx(), [](int idx) { return true; });

            create_output_directory("DM", xy, "Vx", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vy) {
            //-----------------------Вычисление значений Vy--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, data.get_vy(), [](int idx) { return true; });

            create_output_directory("DM", xy, "Vy", files_size);
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vz) {
            //-----------------------Вычисление значений Vz--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, data.get_vz(), [](int idx) { return true; });
            create_output_directory("DM", xy, "Vz", files_size);
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_V) {
            //-----------------------Вычисление значений |V|--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_module(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("DM", xy, "V_module", files_size);
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_c_r) {
            //-----------------------Вычисление значений c_r--------------------------
            Z.push_back(std::vector<double>());
            calculate_c_r(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("DM", xy, "c_r", files_size);
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_c_phi) {
            //-----------------------Вычисление значений c_phi--------------------------
            Z.push_back(std::vector<double>());
            calculate_c_phi(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("DM", xy, "c_phi", files_size);
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_c_z) {
            //-----------------------Вычисление значений c_z--------------------------
            Z.push_back(std::vector<double>());
            calculate_c_z(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("DM", xy, "c_z", files_size);
        } else {
            // TODO: добавить класс логов
            return;
        }
    }
}
