#pragma once
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <vector>

#include "IConverter_Hydrodinamics.h"

template <class Predicate>
void IConverter_Hydrodinamics::calculate_LgSigma(std::vector<double>& projection1, std::vector<double>& projection2,
                                                 std::vector<double>& projection3, const Predicate& condition)
{
    {
        const auto& Ns = data.get_Ns();  // число строк в каждом файле
        const auto& m_p = data.get_m();
        const auto& offests = data.get_offsets();
        Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
        //------------------------------------вычисления значений на сетке----------------------------------
        for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
            size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
            for (int j = 0; j < Ns[i]; ++j) {
                size_t index = offests[i] - offests[0] + j;
                if (condition(index)) {
                    double pr1 = projection1[index];
                    double pr2 = projection2[index];
                    double pr3 = projection3[index];
                    auto [pr1_area, pr2_area] = SC_Area.new_coordinate(pr1, pr2, pr3);
                    if (pr1_area >= limits_x.min && pr1_area <= limits_x.max && pr2_area >= limits_y.min &&
                        pr2_area <= limits_y.max) {
                        int ib = static_cast<int>((pr1_area - limits_x.min) / hb);
                        int jb = static_cast<int>((pr2_area - limits_y.min) / hb);
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
}

template <class Predicate>
void IConverter_Hydrodinamics::calculate_LgRho(std::vector<double>& projection1, std::vector<double>& projection2,
                                               std::vector<double>& projection3, const Predicate& condiditon)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& rho_p = data.get_rho();
    const auto& offests = data.get_offsets();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (condiditon(index)) {
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
                        Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += rho_p[index];
                        Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
                    }
                }
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        if (Nij_b[i] != 0) {
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
        limits_f.back().push_back({std::log10(fmax * l_rho), std::log10(0.000001 * fmax * l_rho)});
    }
    //-----------------------------------перевод к размерным величинам----------------------------------
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        Z.back()[i] = std::log10(Z.back()[i] * l_rho + eps_lg);
    }
}
template <class Predicate>
void IConverter_Hydrodinamics::calculate_Rho(std::vector<double>& projection1, std::vector<double>& projection2,
                                             std::vector<double>& projection3, const Predicate& condiditon)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& rho_p = data.get_rho();
    const auto& offests = data.get_offsets();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (condiditon(index)) {
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
                        Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += rho_p[index];
                        Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
                    }
                }
            }
        }
    }
#pragma omp parallel for simd
    for (size_t i = 0; i < Z.back().size(); ++i) {
        if (Nij_b[i] != 0) {
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
template <class Predicate>
void IConverter_Hydrodinamics::calculate_T(std::vector<double>& projection1, std::vector<double>& projection2,
                                           std::vector<double>& projection3, const Predicate& condiditon)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& offests = data.get_offsets();
    const auto& e = data.get_e();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (condiditon(index)) {
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
                        Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += e[index];
                        Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
                    }
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
template <class Predicate>
void IConverter_Hydrodinamics::calculate_LgT(std::vector<double>& projection1, std::vector<double>& projection2,
                                             std::vector<double>& projection3, const Predicate& condiditon)
{
    const auto& Ns = data.get_Ns();  // число строк в каждом файле
    const auto& offests = data.get_offsets();
    const auto& e = data.get_e();
    std::vector<size_t> Nij_b;
    //------------------------------------вычисления значений на сетке----------------------------------
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (condiditon(index)) {
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
                        Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += e[index];
                        Nij_b[ib + jb * Nb_XY.Ny + Z_offset]++;
                    }
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
