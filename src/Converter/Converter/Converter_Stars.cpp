#include "Converter/Converter_Stars.h"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <vector>

void Converter_Stars::convert()
{
    Z.reserve(Z_grd_list.size());
    auto [projection1, projection2, projection3, v_projection1, v_projection2] = get_projection();

    size_t files_size = data.get_last_file_names().size();
    ofiles_names.reserve(files_size * Z_grd_list.size());

    std::string xy = extract_upper_axis(std::string(XY.first)) + extract_upper_axis(std::string(XY.second));
    for (size_t file_type = 0; file_type < Z_grd_list.size(); ++file_type) {
        if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Rho) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Rho(projection1, projection2, projection3);
            create_output_directory("STARS", xy, "Rho", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgRho) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgRho(projection1, projection2, projection3);
            create_output_directory("STARS", xy, "LgRho", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgSigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgSigma(projection1, projection2, projection3);
            create_output_directory("STARS", xy, "LgSigma", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Sigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Sigma(projection1, projection2, projection3, [](int idx) { return true; });
            create_output_directory("STARS", xy, "Sigma", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vfi) {  // TODO: неправильное преобразование
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Vfi(projection1, projection2, projection3, v_projection1, v_projection2);
            create_output_directory("STARS", xy, "Vfi", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vr) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Vr(projection1, projection2, projection3, v_projection1, v_projection2);
            create_output_directory("STARS", xy, "Vr", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vx) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, data.get_vx(), [](int idx) { return true; });

            create_output_directory("STARS", xy, "Vx", files_size);

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vy) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, data.get_vy(), [](int idx) { return true; });

            create_output_directory("STARS", xy, "Vy", files_size);
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vz) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, data.get_vz(), [](int idx) { return true; });
            create_output_directory("STARS", xy, "Vz", files_size);
        } else {
            // TODO: добавить логи
            return;
        }
    }
}
void Converter_Stars::calculate_Vr(std::vector<double>& projection1, std::vector<double>& projection2,
                                   std::vector<double>& projection3, std::vector<double>& v_projection1,
                                   std::vector<double>& v_projection2)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& offests = data.get_offsets();

    std::vector<size_t> Nij_b;
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            double pr1 = projection1[index];
            double pr2 = projection2[index];
            double pr3 = projection3[index];
            auto [new_pr1, new_pr2, new_pr3] = SC_rectangle.new_coordinate(pr1, pr2, pr3);
            if (new_pr1 >= bound_x.min && new_pr1 <= bound_x.max && new_pr2 >= bound_y.min && new_pr2 <= bound_y.max &&
                new_pr3 >= bound_z.min && new_pr3 <= bound_z.max) {
                auto [pr1_area, pr2_area] = SC_Area.new_coordinate(pr1, pr2, pr3);
                if (pr1_area >= limits_x.min && pr1_area <= limits_x.max && pr2_area >= limits_y.min &&
                    pr2_area <= limits_y.max) {
                    int ib = static_cast<int>((pr1_area - limits_x.min) / hb);
                    int jb = static_cast<int>((pr2_area - limits_y.min) / hb);
                    double vpr1 = v_projection1[index];
                    double vpr2 = v_projection2[index];
                    double r = std::sqrt(pr1 * pr1 + pr2 * pr2);
                    double vr = (r > 0.0) ? (vpr1 * pr1 + vpr2 * pr2) / r : 0.0;
                    Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += vr;
                    Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
                }
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        if (Z.back()[i] != 0) {
            Z.back()[i] /= Nij_b[i];
        }
    }
    //----------------------------------вычисление предельных значений---------------------------------
    limits_f.push_back(std::vector<lim<double>>());
    limits_f.back().reserve(Nfiles_into_clomun);
    for (size_t i = 0; i < Nfiles_into_clomun; ++i) {
        size_t index = Nb_XY.Nx * Nb_XY.Ny * i;
        double fmax = Z.back()[index];
        double fmin = Z.back()[index];
        for (int j = 0; j < Nb_XY.Nx * Nb_XY.Ny; ++j) {
            fmax = (Z.back()[index + j] > fmax) ? Z.back()[index + j] : fmax;
            fmin = (Z.back()[index + j] < fmin) ? Z.back()[index + j] : fmin;
        }
        limits_f.back().push_back({fmax * l_v, fmin * l_v});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = Z.back()[i] * l_v;
    }
}
void Converter_Stars::calculate_Vfi(std::vector<double>& projection1, std::vector<double>& projection2,
                                    std::vector<double>& projection3, std::vector<double>& v_projection1,
                                    std::vector<double>& v_projection2)
{
    // число строк в каждом файле
    const auto& Ns = data.get_Ns();
    const auto& offests = data.get_offsets();

    std::vector<size_t> Nij_b;
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            double pr1 = projection1[index];
            double pr2 = projection2[index];
            double pr3 = projection3[index];
            auto [new_pr1, new_pr2, new_pr3] = SC_rectangle.new_coordinate(pr1, pr2, pr3);
            if (new_pr1 >= bound_x.min && new_pr1 <= bound_x.max && new_pr2 >= bound_y.min && new_pr2 <= bound_y.max &&
                new_pr3 >= bound_z.min && new_pr3 <= bound_z.max) {
                auto [pr1_area, pr2_area] = SC_Area.new_coordinate(pr1, pr2, pr3);
                if (pr1_area >= limits_x.min && pr1_area <= limits_x.max && pr2_area >= limits_y.min &&
                    pr2_area <= limits_y.max) {
                    int ib = static_cast<int>((pr1_area - limits_x.min) / hb);
                    int jb = static_cast<int>((pr2_area - limits_y.min) / hb);
                    double vpr1 = v_projection1[index];
                    double vpr2 = v_projection2[index];
                    double r = std::sqrt(pr1 * pr1 + pr2 * pr2);
                    double vfi = (r > 0.0) ? (vpr2 * pr1 - vpr1 * pr1) / r : 0.0;
                    Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += vfi;
                    Nij_b[ib * Nb_XY.Nx + jb + Z_offset]++;
                }
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        if (Z.back()[i] != 0) {
            Z.back()[i] /= Nij_b[i];
        }
    }
    //----------------------------------вычисление предельных значений---------------------------------
    limits_f.push_back(std::vector<lim<double>>());
    limits_f.back().reserve(Nfiles_into_clomun);
    for (size_t i = 0; i < Nfiles_into_clomun; ++i) {
        size_t index = Nb_XY.Nx * Nb_XY.Ny * i;
        double fmax = Z.back()[index];
        double fmin = Z.back()[index];
        for (int j = 0; j < Nb_XY.Nx * Nb_XY.Ny; ++j) {
            fmax = (Z.back()[index + j] > fmax) ? Z.back()[index + j] : fmax;
            fmin = (Z.back()[index + j] < fmin) ? Z.back()[index + j] : fmin;
        }
        limits_f.back().push_back({fmax * l_v, fmin * l_v});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = Z.back()[i] * l_v;
    }
}