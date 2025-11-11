#include "Converter/Converter_YoungStars.h"

#include <cmath>
Converter_YoungStars::Converter_YoungStars(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy,
                                           std::filesystem::path _output_path)
    : Converter(_data, c, _Nbxy, _output_path)
{
    gamma = c.gamma;
    gamma1 = c.gamma - 1.0;
    l_Te = gamma * gamma1 * 10000 / (100.0 / (l_v * l_v));
}
void Converter_YoungStars::convert()
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
        } else if (xy->second == ParametrsList::Y) {
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
        } else if (xy->second == ParametrsList::Y) {
            return data->get_vy();
        } else {
            return data->get_vz();
        }
    }();

    auto file_pathes = data.get_last_file_names();
    std::string xy = extract_upper_axis(std::string(XY.first)) + extract_upper_axis(std::string(XY.second));

    std::vector<std::string> file_names;
    file_names.reserve(file_pathes.size());
    for (const auto& i : file_pathes) {
        file_names.push_back(i.stem().string());
    }

    for (size_t file_type = 0; file_type < Z_grd_list.size(); ++file_type) {
        if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Rho) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Rho(projection1, projection2, projection3);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Rho";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgRho) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgRho(projection1, projection2, projection3);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "LgRho";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_LgSigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgSigma(projection1, projection2);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "LgSigma";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Sigma) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Sigma(projection1, projection2);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Sigma";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vfi) {  // TODO: неправильная работа
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Vfi(projection1, projection2, projection3, v_projection1, v_projection2);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Vfi";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vr) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_Vr(projection1, projection2, projection3, v_projection1, v_projection2);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Vr";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vx) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, data.get_vx());
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Vx";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vy) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, data.get_vy());
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Vy";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParams_Vz) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_V_projection(projection1, projection2, projection3, data.get_vz());
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Vz";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }

        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParamT) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_T(projection1, projection2, projection3);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "Termal";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }
        } else if (Z_grd_list[file_type] == ParametrsList::Z_outParamLgT) {
            //-----------------------Вычисление значений--------------------------
            Z.push_back(std::vector<double>());
            calculate_LgT(projection1, projection2, projection3);
            //-----------------------создание поддиректории--------------------------
            ofiles_names.reserve(file_names.size());
            std::string dir = "LgTermal";
            std::filesystem::create_directory(output_directory / dir);
            for (size_t i = 0; i < file_names.size(); ++i) {
                ofiles_names.push_back(output_directory / dir /
                                       ("YS_" + xy + "_" + dir + "_t_" + std::to_string(data.get_t()[i] * l_t) + ".grd"));
            }
        } else {
            // TODO: добавить логи
            return;
        }
    }
}
void Converter_YoungStars::calculate_LgSigma(std::vector<double>& projection1, std::vector<double>& projection2)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& m_p = data.get_m();
    const auto& offests = data.get_offsets();
    const auto& ind_sph = data.get_ind_sph();
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 2) {
                double pr1 = projection1[index];
                double pr2 = projection2[index];
                if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max) {
                    int ib = (int)((pr1 - limits_x.min) / hb);
                    int jb = (int)((pr2 - limits_y.min) / hb);
                    Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += m_p[index];
                }
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
        limits_f.back().push_back({std::log10(fmax * _hb2 * l_s), std::log10(0.000001 * fmax * l_s * _hb2)});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = std::log10(Z.back()[i] * l_s * _hb2 + eps_lg);
    }
}
void Converter_YoungStars::calculate_Sigma(std::vector<double>& projection1, std::vector<double>& projection2)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& m_p = data.get_m();
    const auto& offests = data.get_offsets();
    const auto& ind_sph = data.get_ind_sph();
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 2) {
                double pr1 = projection1[index];
                double pr2 = projection2[index];
                if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max) {
                    int ib = (int)((pr1 - limits_x.min) / hb);
                    int jb = (int)((pr2 - limits_y.min) / hb);
                    Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += m_p[index];
                }
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
void Converter_YoungStars::calculate_LgRho(std::vector<double>& projection1, std::vector<double>& projection2,
                                           std::vector<double>& projection3)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& rho_p = data.get_rho();
    const auto& offests = data.get_offsets();
    const auto& ind_sph = data.get_ind_sph();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 2) {
                double pr1 = projection1[index];
                double pr2 = projection2[index];
                double pr3 = projection3[index];
                if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max &&
                    pr3 >= limits_z.min && pr3 <= limits_z.max) {
                    int ib = (int)((pr1 - limits_x.min) / hb);
                    int jb = (int)((pr2 - limits_y.min) / hb);
                    Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += rho_p[index];
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
        limits_f.back().push_back({std::log10(fmax * l_rho), std::log10(0.000001 * fmax * l_rho)});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = std::log10(Z.back()[i] * l_rho + eps_lg);
    }
}
void Converter_YoungStars::calculate_Rho(std::vector<double>& projection1, std::vector<double>& projection2,
                                         std::vector<double>& projection3)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& rho_p = data.get_rho();
    const auto& offests = data.get_offsets();
    const auto& ind_sph = data.get_ind_sph();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 2) {
                double pr1 = projection1[index];
                double pr2 = projection2[index];
                double pr3 = projection3[index];
                if (pr1 >= limits_x.min && pr1 <= limits_x.max && pr2 >= limits_y.min && pr2 <= limits_y.max &&
                    pr3 >= limits_z.min && pr3 <= limits_z.max) {
                    int ib = (int)((pr1 - limits_x.min) / hb);
                    int jb = (int)((pr2 - limits_y.min) / hb);
                    Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += rho_p[index];
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
void Converter_YoungStars::calculate_Vr(std::vector<double>& projection1, std::vector<double>& projection2,
                                        std::vector<double>& projection3, std::vector<double>& v_projection1,
                                        std::vector<double>& v_projection2)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& offests = data.get_offsets();
    const auto& ind_sph = data.get_ind_sph();
    std::vector<size_t> Nij_b;
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 2) {
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
void Converter_YoungStars::calculate_Vfi(std::vector<double>& projection1, std::vector<double>& projection2,
                                         std::vector<double>& projection3, std::vector<double>& v_projection1,
                                         std::vector<double>& v_projection2)
{
    // число строк в каждом файле
    const auto& Ns = data.get_Ns();
    const auto& offests = data.get_offsets();
    const auto& ind_sph = data.get_ind_sph();

    std::vector<size_t> Nij_b;
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 2) {
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
void Converter_YoungStars::calculate_V_projection(std::vector<double>& projection1, std::vector<double>& projection2,
                                                  std::vector<double>& projection3, std::vector<double>& v_projection)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& offests = data.get_offsets();
    const auto& ind_sph = data.get_ind_sph();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 2) {
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
void Converter_YoungStars::calculate_T(std::vector<double>& projection1, std::vector<double>& projection2,
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
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 2) {
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
void Converter_YoungStars::calculate_LgT(std::vector<double>& projection1, std::vector<double>& projection2,
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
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (ind_sph[index] == 2) {
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