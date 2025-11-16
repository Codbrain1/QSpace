#include "Converter/Converter_MolecularClouds.h"

#include <cmath>
void Converter_MolecularClouds::convert()
{
    Z.reserve(Z_grd_list.size());
    auto [projection1, projection2, projection3] = get_projection();

    size_t files_size = data.get_last_file_names().size();
    ofiles_names.reserve(files_size * Z_grd_list.size());
    std::string xy = extract_upper_axis(std::string(XY.first)) + extract_upper_axis(std::string(XY.second));

    for (size_t file_type = 0; file_type < Z_grd_list.size(); ++file_type) {
        if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Rho) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_Rho(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "Rho", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgRho) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_LgRho(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "LgRho", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgSigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_LgSigma(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "LgSigma", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Sigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_Sigma(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "Sigma", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vfi) {  // неправильная работа
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_Vfi(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "Vfi", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vr) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_Vr(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "Vr", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vx) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_V_projection(projection1, projection2, projection3, data.get_vx(),
                                   [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "Vx", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vy) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_V_projection(projection1, projection2, projection3, data.get_vy(),
                                   [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "Vy", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vz) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_V_projection(projection1, projection2, projection3, data.get_vz(),
                                   [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "Vz", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParamT) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_T(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "Termal", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParamLgT) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_LgT(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 1; });
            create_output_directory("MC", xy, "LgTermal", files_size);

        } else {
            // TODO: добавить логи
            return;
        }
    }
}