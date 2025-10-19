#include "Converter/Converter_Gas.h"

#include <cmath>
Converter_Gas::Converter_Gas(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy,
                             std::filesystem::path _output_path)
    : Converter(_data, c, _Nbxy, _output_path)
{
    gamma = c.gamma;
    gamma1 = c.gamma - 1.0;
    l_Te = l_Te = gamma * gamma1 * 10000 / (100.0 / (l_v * l_v));
}
void Converter_Gas::convert()
{
    Z.reserve(Z_grd_list.size());
    std::vector<double>& projection1 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для x
        if (xy->first == ParametrsList::X) {
            return data->get_x();
        } else if (xy->first == ParametrsList::Y) {
            return data->get_y();
        } else {
            return data->get_z();
        }
    }();

    std::vector<double>& projection2 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для y
        if (xy->second == ParametrsList::X) {
            return data->get_x();
        } else if (xy->first == ParametrsList::Y) {
            return data->get_y();
        } else {
            return data->get_z();
        }
    }();
    std::vector<double>& projection3 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для z
        if ((xy->first == ParametrsList::X && xy->second == ParametrsList::Y) ||
            (xy->first == ParametrsList::Y && xy->second == ParametrsList::X)) {
            return data->get_z();
        } else if ((xy->first == ParametrsList::X && xy->second == ParametrsList::Z) ||
                   (xy->first == ParametrsList::Z && xy->second == ParametrsList::X)) {
            return data->get_y();
        } else {
            return data->get_x();
        }
    }();

    std::vector<double>& v_projection1 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для x
        if (xy->first == ParametrsList::X) {
            return data->get_vx();
        } else if (xy->first == ParametrsList::Y) {
            return data->get_vy();
        } else {
            return data->get_vz();
        }
    }();

    std::vector<double>& v_projection2 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для y
        if (xy->second == ParametrsList::X) {
            return data->get_vx();
        } else if (xy->first == ParametrsList::Y) {
            return data->get_vy();
        } else {
            return data->get_vz();
        }
    }();
    std::vector<double>& v_projection3 = [xy = &XY, data = &data]() -> std::vector<double>& {  // проекция для z
        if ((xy->first == ParametrsList::X && xy->second == ParametrsList::Y) ||
            (xy->first == ParametrsList::Y && xy->second == ParametrsList::X)) {
            return data->get_vz();
        } else if ((xy->first == ParametrsList::X && xy->second == ParametrsList::Z) ||
                   (xy->first == ParametrsList::Z && xy->second == ParametrsList::X)) {
            return data->get_vy();
        } else {
            return data->get_vx();
        }
    }();
    auto file_pathes = data.get_last_file_names();
    std::vector<std::string> file_names(file_pathes.size());
    for (const auto& i : file_pathes) {
        file_names.push_back(i.stem());
    }

    for (size_t file_type = 0; file_type < Z_grd_list.size(); ++file_type) {
        if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Rho) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Rho(projection1, projection2, projection3);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Rho";
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgRho) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgRho(projection1, projection2, projection3);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "LgRho";
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgSigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgSigma(projection1, projection2);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "LgSigma";
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Sigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Sigma(projection1, projection2);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Sigma";
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vfi) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Vfi(projection1, projection2, projection3, v_projection1, v_projection2);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Vfi";
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vr) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Vr(projection1, projection2, projection3, v_projection1, v_projection2);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Vr";
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vx) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, v_projection1);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = std::string(XY.first);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vy) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, v_projection2);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = std::string(XY.second);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vz) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, v_projection3);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = [xy = &XY]() {  // проекция для z
                if ((xy->first == ParametrsList::X && xy->second == ParametrsList::Y) ||
                    (xy->first == ParametrsList::Y && xy->second == ParametrsList::X)) {
                    return std::string(ParametrsList::Z_outParams_Vz);
                } else if ((xy->first == ParametrsList::X && xy->second == ParametrsList::Z) ||
                           (xy->first == ParametrsList::Z && xy->second == ParametrsList::X)) {
                    return std::string(ParametrsList::Z_outParams_Vy);
                } else {
                    return std::string(ParametrsList::Z_outParams_Vx);
                }
            }();
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParamT) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_T(projection1, projection2, projection3);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Termal";
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParamLgT) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgT(projection1, projection2, projection3);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "LgTermal";
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir / (dir + file_names[i]));
            }
        } else {
            // TODO: добавить логи
            return;
        }
    }
}
void Converter_Gas::calculate_LgSigma(std::vector<double>& projection1, std::vector<double>& projection2)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& m_p = data.get_m();
    const auto& offests = data.get_offsets();
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    for (size_t i = 0; i < Ns.size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (size_t j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            double pr1 = projection1[index];
            double pr2 = projection2[index];
            if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max) {
                int ib = (int)((pr1 - limits_x.min) / hb);
                int jb = (int)((pr2 - limits_y.min) / hb);
                Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += m_p[index];
            }
        }
    }
    //----------------------------------вычисление предельных значений---------------------------------
    limits_f.push_back(std::vector<lim<double>>());
    limits_f.back().reserve(Nfiles_into_clomun);
    for (size_t i = 0; i < Nfiles_into_clomun; ++i) {
        size_t index = Nb_XY.Nx * Nb_XY.Ny * i;
        double fmax = Z.back()[index];
        for (int j = 0; j < Nb_XY.Nx * Nb_XY.Ny; ++j) {
            fmax = (Z.back()[index + j] > fmax) ? Z.back()[index + j] : fmax;
        }
        limits_f.back().push_back({std::log10(fmax * _hb2 * l_s), std::log10(0.0001 * fmax * l_s)});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = std::log10(Z.back()[i] * l_s * _hb2 + eps_lg);
    }
}
void Converter_Gas::calculate_Sigma(std::vector<double>& projection1, std::vector<double>& projection2)
{
    auto Ns = data.get_Ns();  // число строк в каждом файле
    auto m_p = data.get_m();
    auto offests = data.get_offsets();
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    for (size_t i = 0; i < Ns.size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (size_t j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            double pr1 = projection1[index];
            double pr2 = projection2[index];
            if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max) {
                int ib = (int)((pr1 - limits_x.min) / hb);
                int jb = (int)((pr2 - limits_y.min) / hb);
                Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += m_p[index];
            }
        }
    }
    //----------------------------------вычисление предельных значений---------------------------------
    limits_f.push_back(std::vector<lim<double>>());
    limits_f.back().reserve(Nfiles_into_clomun);
    for (size_t i = 0; i < Nfiles_into_clomun; ++i) {
        size_t index = Nb_XY.Nx * Nb_XY.Ny * i;
        double fmax = Z.back()[index];
        double fmin = 0;
        for (int j = 0; j < Nb_XY.Nx * Nb_XY.Ny; ++j) {
            fmax = (Z.back()[index + j] > fmax) ? Z.back()[index + j] : fmax;
        }
        limits_f.back().push_back({fmax * l_s * _hb2, fmin});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] *= l_s * _hb2;
    }
}
void Converter_Gas::calculate_LgRho(std::vector<double>& projection1, std::vector<double>& projection2,
                                    std::vector<double>& projection3)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& m_p = data.get_m();
    const auto& offests = data.get_offsets();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < Ns.size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (size_t j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            double pr1 = projection1[index];
            double pr2 = projection2[index];
            double pr3 = projection3[index];
            if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max &&
                pr3 >= limits_z.min && pr3 <= limits_z.max) {
                int ib = (int)((pr1 - limits_x.min) / hb);
                int jb = (int)((pr2 - limits_y.min) / hb);
                Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += m_p[index];
                Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] /= Nij_b[i];
    }
    //----------------------------------вычисление предельных значений---------------------------------
    limits_f.push_back(std::vector<lim<double>>());
    limits_f.back().reserve(Nfiles_into_clomun);
    for (size_t i = 0; i < Nfiles_into_clomun; ++i) {
        size_t index = Nb_XY.Nx * Nb_XY.Ny * i;
        double fmax = Z.back()[index];
        for (int j = 0; j < Nb_XY.Nx * Nb_XY.Ny; ++j) {
            fmax = (Z.back()[index + j] > fmax) ? Z.back()[index + j] : fmax;
        }
        limits_f.back().push_back({std::log10(fmax * l_rho), std::log10(0.0001 * fmax * l_rho)});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = std::log10(Z.back()[i] * l_rho + eps_lg);
    }
}
void Converter_Gas::calculate_Rho(std::vector<double>& projection1, std::vector<double>& projection2,
                                  std::vector<double>& projection3)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& m_p = data.get_m();
    const auto& offests = data.get_offsets();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < Ns.size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (size_t j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            double pr1 = projection1[index];
            double pr2 = projection2[index];
            double pr3 = projection3[index];
            if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max &&
                pr3 >= limits_z.min && pr3 <= limits_z.max) {
                int ib = (int)((pr1 - limits_x.min) / hb);
                int jb = (int)((pr2 - limits_y.min) / hb);
                Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += m_p[index];
                Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] /= Nij_b[i];
    }
    //----------------------------------вычисление предельных значений---------------------------------
    limits_f.push_back(std::vector<lim<double>>());
    limits_f.back().reserve(Nfiles_into_clomun);
    for (size_t i = 0; i < Nfiles_into_clomun; ++i) {
        size_t index = Nb_XY.Nx * Nb_XY.Ny * i;
        double fmax = Z.back()[index];
        double fmin = 0;
        for (int j = 0; j < Nb_XY.Nx * Nb_XY.Ny; ++j) {
            fmax = (Z.back()[index + j] > fmax) ? Z.back()[index + j] : fmax;
        }
        limits_f.back().push_back({fmax * l_rho, fmin});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = Z.back()[i] * l_rho;
    }
}
void Converter_Gas::calculate_Vr(std::vector<double>& projection1, std::vector<double>& projection2,
                                 std::vector<double>& projection3, std::vector<double>& v_projection1,
                                 std::vector<double>& v_projection2)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& offests = data.get_offsets();

    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < Ns.size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (size_t j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            double pr1 = projection1[index];
            double pr2 = projection2[index];
            double pr3 = projection3[index];
            if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max &&
                pr3 >= limits_z.min && pr3 <= limits_z.max) {
                int ib = (int)((pr1 - limits_x.min) / hb);
                int jb = (int)((pr2 - limits_y.min) / hb);
                double vpr1 = v_projection1[index];
                double vpr2 = v_projection2[index];
                double r = std::sqrt(pr1 * pr1 + pr2 * pr2);
                double vr = (r > 0.0) ? (vpr1 * pr1 + vpr2 * pr2) / r : 0.0;
                Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += vr;
                Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] /= Nij_b[i];
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
void Converter_Gas::calculate_Vfi(std::vector<double>& projection1, std::vector<double>& projection2,
                                  std::vector<double>& projection3, std::vector<double>& v_projection1,
                                  std::vector<double>& v_projection2)
{
    // число строк в каждом файле
    const auto& Ns = data.get_Ns();
    const auto& offests = data.get_offsets();

    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < Ns.size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (size_t j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            double pr1 = projection1[index];
            double pr2 = projection2[index];
            double pr3 = projection3[index];
            if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max &&
                pr3 >= limits_z.min && pr3 <= limits_z.max) {
                int ib = (int)((pr1 - limits_x.min) / hb);
                int jb = (int)((pr2 - limits_y.min) / hb);
                double vpr1 = v_projection1[index];
                double vpr2 = v_projection2[index];
                double r = std::sqrt(pr1 * pr1 + pr2 * pr2);
                double vfi = (r > 0.0) ? (vpr2 * pr1 - vpr1 * pr1) / r : 0.0;
                Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += vfi;
                Nij_b[ib * Nb_XY.Nx + jb + Z_offset]++;
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] /= Nij_b[i];
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
void Converter_Gas::calculate_V_projection(std::vector<double>& projection1, std::vector<double>& projection2,
                                           std::vector<double>& projection3, std::vector<double>& v_projection)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& offests = data.get_offsets();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < Ns.size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (size_t j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            double pr1 = projection1[index];
            double pr2 = projection2[index];
            double pr3 = projection3[index];
            if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max &&
                pr3 >= limits_z.min && pr3 <= limits_z.max) {
                int ib = (int)((pr1 - limits_x.min) / hb);
                int jb = (int)((pr2 - limits_y.min) / hb);
                Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += v_projection[index];
                Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] /= Nij_b[i];
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
void Converter_Gas::calculate_T(std::vector<double>& projection1, std::vector<double>& projection2,
                                std::vector<double>& projection3)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& offests = data.get_offsets();
    const auto& e = data.get_e();
    const auto& ind_sph = data.get_ind_sph();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < Ns.size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (size_t j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 0) {
                double pr1 = projection1[index];
                double pr2 = projection2[index];
                double pr3 = projection3[index];
                if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max &&
                    pr3 >= limits_z.min && pr3 <= limits_z.max) {
                    int ib = (int)((pr1 - limits_x.min) / hb);
                    int jb = (int)((pr2 - limits_y.min) / hb);
                    Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += e[index];
                    Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
                }
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] /= Nij_b[i];
    }
    //----------------------------------вычисление предельных значений---------------------------------
    limits_f.push_back(std::vector<lim<double>>());
    limits_f.back().reserve(Nfiles_into_clomun);
    for (size_t i = 0; i < Nfiles_into_clomun; ++i) {
        size_t index = Nb_XY.Nx * Nb_XY.Ny * i;
        double fmax = Z.back()[index];
        double fmin = 0;
        for (int j = 0; j < Nb_XY.Nx * Nb_XY.Ny; ++j) {
            fmax = (Z.back()[index + j] > fmax) ? Z.back()[index + j] : fmax;
        }
        limits_f.back().push_back({fmax * l_Te, fmin});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = Z.back()[i] * l_Te;
    }
}
void Converter_Gas::calculate_LgT(std::vector<double>& projection1, std::vector<double>& projection2,
                                  std::vector<double>& projection3)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& offests = data.get_offsets();
    const auto& e = data.get_e();
    const auto& ind_sph = data.get_ind_sph();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < Ns.size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (size_t j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 0) {
                double pr1 = projection1[index];
                double pr2 = projection2[index];
                double pr3 = projection3[index];
                if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max &&
                    pr3 >= limits_z.min && pr3 <= limits_z.max) {
                    int ib = (int)((pr1 - limits_x.min) / hb);
                    int jb = (int)((pr2 - limits_y.min) / hb);
                    Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += e[index];
                    Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
                }
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] /= Nij_b[i];
    }
    //----------------------------------вычисление предельных значений---------------------------------
    limits_f.push_back(std::vector<lim<double>>());
    limits_f.back().reserve(Nfiles_into_clomun);
    for (size_t i = 0; i < Nfiles_into_clomun; ++i) {
        size_t index = Nb_XY.Nx * Nb_XY.Ny * i;
        double fmax = Z.back()[index];
        for (int j = 0; j < Nb_XY.Nx * Nb_XY.Ny; ++j) {
            fmax = (Z.back()[index + j] > fmax) ? Z.back()[index + j] : fmax;
        }
        limits_f.back().push_back({log10(fmax * l_Te + eps_lg), std::log10(0.000001 * fmax * l_Te + eps_lg)});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = log10(Z.back()[i] * l_Te + eps_lg);
    }
}