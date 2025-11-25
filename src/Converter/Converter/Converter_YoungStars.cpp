#include "Converter/Converter_YoungStars.h"

#include <cmath>
void Converter_YoungStars::convert()
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
            calculate_Rho(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "Rho", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgRho) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_LgRho(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "LgRho", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgSigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_LgSigma(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "LgSigma", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Sigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_Sigma(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "Sigma", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vfi) {  // неправильная работа
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_Vfi(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "Vfi", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vr) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_Vr(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "Vr", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vx) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_V_projection(projection1, projection2, projection3, data.get_vx(),
                                   [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "Vx", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vy) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_V_projection(projection1, projection2, projection3, data.get_vy(),
                                   [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "Vy", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vz) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_V_projection(projection1, projection2, projection3, data.get_vz(),
                                   [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "Vz", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParamT) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_T(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "Termal", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParamLgT) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            const auto& ind_sph = data.get_ind_sph();
            calculate_LgT(projection1, projection2, projection3, [&ind_sph](int idx) { return ind_sph[idx] == 2; });
            create_output_directory("YS", xy, "LgTermal", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_V) {
            //-----------------------Вычисление значений |V|--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_module(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("YS", xy, "V_module", files_size);
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_c_r) {
            //-----------------------Вычисление значений c_r--------------------------
            Z.push_back(std::vector<double>());
            calculate_c_r(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("YS", xy, "c_r", files_size);
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_c_phi) {
            //-----------------------Вычисление значений c_phi--------------------------
            Z.push_back(std::vector<double>());
            calculate_c_phi(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("YS", xy, "c_phi", files_size);
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_c_z) {
            //-----------------------Вычисление значений c_z--------------------------
            Z.push_back(std::vector<double>());
            calculate_c_z(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("YS", xy, "c_z", files_size);
        } else {
            // TODO: добавить класс логов
            return;
        }
    }
}
