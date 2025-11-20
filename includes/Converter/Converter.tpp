#pragma once
#include "Converter.h"

template <class Predicate>
void Converter::calculate_V_projection(std::vector<double>& projection1, std::vector<double>& projection2,
                                       std::vector<double>& projection3, std::vector<double>& v_projection,
                                       const Predicate& condition)
{
    {
        const auto& Ns = data.get_Ns();  // число строк в каждом
                                         // файле
        const auto& offests = data.get_offsets();
        std::vector<size_t> Nij_b;
        //------------------------------------вычисления значений на сетке----------------------------------
        Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
        Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
        for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
            size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
            for (int j = 0; j < Ns[i]; ++j) {
                size_t index = offests[i] - offests[0] + j;
                if (condition(index)) {
                    double pr1 = projection1[index];
                    double pr2 = projection2[index];
                    double pr3 = projection3[index];
                    auto [new_pr1, new_pr2, new_pr3] = SC_rectangle.new_coordinate(pr1, pr2, pr3);
                    if (new_pr1 >= bound_x.min && new_pr1 <= bound_x.max && new_pr2 >= bound_y.min &&
                        new_pr2 <= bound_y.max && new_pr3 >= bound_z.min && new_pr3 <= bound_z.max) {
                        auto [pr1_area, pr2_area] = SC_Area.new_coordinate(pr1, pr2, pr3);
                        if (pr1_area >= limits_x.min && pr1_area <= limits_x.max && pr2_area >= limits_y.min &&
                            pr2_area <= limits_y.max) {
                            int ib = static_cast<int>((pr1_area - limits_x.min) / hb);
                            int jb = static_cast<int>((pr2_area - limits_y.min) / hb);
                            Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += v_projection[index];
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
}

template <class Predicate>
void Converter::calculate_Sigma(std::vector<double>& projection1, std::vector<double>& projection2,
                                std::vector<double>& projection3, const Predicate& condition)
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

template <class Predicate>
std::array<double, 3> Converter::calculate_Center_of_Mass(int i_file, const Predicate& condition)
{
    const auto& x = data.get_x();
    const auto& y = data.get_y();
    const auto& z = data.get_z();
    const auto& Ns = data.get_Ns();
    const auto& offests = data.get_offsets();
    const auto& m_p = data.get_m();
    std::array<double, 3> centers_of_mass{0, 0, 0};
    double sum_mass = 0;
    double c_x = 0, c_y = 0, c_z = 0;
//------------------------------------вычисления значений по компонентам x,y,z----------------------------------
#pragma omp parallel for simd reduction(+ : c_x, c_y, c_z, sum_mass)
    for (int j = 0; j < Ns[i_file]; ++j) {
        size_t index = offests[i_file] - offests[0] + j;
        if (condition(index)) {
            double x_ind = x[index];
            double y_ind = y[index];
            double z_ind = z[index];
            double m_p_ind = m_p[index];
            c_x += m_p_ind * x_ind;
            c_y += m_p_ind * y_ind;
            c_z += m_p_ind * z_ind;
            sum_mass += m_p_ind;
        }
    }

    auto& [cx, cy, cz] = centers_of_mass;
    cx = c_x / sum_mass;
    cy = c_y / sum_mass;
    cz = c_z / sum_mass;
    return centers_of_mass;
}
template <class Predicate>
std::vector<std::array<double, 3>> Converter::calculate_P(const Predicate& condition)  // полный импульс
{
    const auto& vx = data.get_vx();
    const auto& vy = data.get_vy();
    const auto& vz = data.get_vz();
    const auto& Ns = data.get_Ns();
    const auto& offests = data.get_offsets();
    const auto& m_p = data.get_m();
    std::vector<std::array<double, 3>> Impulse;
    Impulse.resize(data.get_ibuff_size(), {0, 0, 0});
//------------------------------------вычисления значений по компонентам x,y,z----------------------------------
#pragma omp parallel for simd schedule(static)
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        double px = 0, py = 0, pz = 0;
#pragma omp simd reduction(+ : px, py, pz)
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (condition(index)) {
                double vx_i = vx[index];
                double vy_i = vy[index];
                double vz_i = vz[index];
                double m_p_i = m_p[index];
                px += m_p_i * vx_i, py += m_p_i * vy_i, pz += m_p_i * vz_i;
            }
        }
        Impulse[i] = {px, py, pz};
    }
    return Impulse;
}
template <class Predicate>
std::vector<std::array<double, 3>> Converter::calculate_L(const Predicate& condition)  // момент
                                                                                       // импульса
{
    const auto& x = data.get_x();
    const auto& y = data.get_y();
    const auto& z = data.get_z();
    const auto& vx = data.get_vx();
    const auto& vy = data.get_vy();
    const auto& vz = data.get_vz();
    const auto& Ns = data.get_Ns();
    const auto& offests = data.get_offsets();
    const auto& m_p = data.get_m();
    std::vector<std::array<double, 3>> moment_impulse;
    moment_impulse.resize(data.get_ibuff_size(), {0, 0, 0});
//------------------------------------вычисления значений по компонентам x,y,z----------------------------------
#pragma omp parallel for simd schedule(static)
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        double L_x = 0;
        double L_y = 0;
        double L_z = 0;
#pragma omp simd reduction(+ : L_x, L_y, L_z)
        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (condition(index)) {
                double x_ind = x[index];
                double y_ind = y[index];
                double z_ind = z[index];
                double vx_ind = vx[index];
                double vy_ind = vy[index];
                double vz_ind = vz[index];
                double m_p_ind = m_p[index];
                L_x += m_p_ind * (y_ind * vz_ind - z_ind * vy_ind);
                L_y += m_p_ind * (z_ind * vx_ind - x_ind * vz_ind);
                L_z += m_p_ind * (x_ind * vy_ind - y_ind * vx_ind);
            }
        }
        moment_impulse[i] = {L_x, L_y, L_z};
    }
    return moment_impulse;
}
template <class Predicate>
void Converter::calculate_Vr(std::vector<double>& projection1, std::vector<double>& projection2,
                             std::vector<double>& projection3, const Predicate& condition)
{
    const auto& moment_impulse = calculate_L(condition);  // вычисление полного момента импульса системы
    // число строк в каждом файле
    const auto& Ns = data.get_Ns();
    const auto& offests = data.get_offsets();
    const auto& x = data.get_x();
    const auto& y = data.get_y();
    const auto& z = data.get_z();
    const auto& vx = data.get_vx();
    const auto& vy = data.get_vy();
    const auto& vz = data.get_vz();
    std::vector<size_t> Nij_b;
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        const auto& [Lx, Ly, Lz] = moment_impulse[i];

        constexpr double EPS = 1e-12;
        constexpr double Unit_Tol = 0.99;

        std::array<double, 3> r0;  // новый центр координат
        r0 = calculate_Center_of_Mass(i, condition);

        std::array<double, 3> e_z;  // вектор нормали к плоскости диска
        double L = std::hypot(Lx, Ly, Lz);
        if (L > EPS) {
            const double div_L = 1.0 / L;
            e_z = {Lx * div_L, Ly * div_L, Lz * div_L};
        } else {
            e_z = {0, 0, 1};
        }

        std::array<double, 3> e_x, e_y;
        const auto [n_x, n_y, n_z] = e_z;

        std::array<double, 3> a;  // вспомогательный вектор для определения новой системы координат
        if (std::abs(n_z) > Unit_Tol) {
            a = {1, 0, 0};
        } else {
            a = {0, 0, 1};
        }
        const auto [ax, ay, az] = a;
        double na_x = n_y * az - n_z * ay;
        double na_y = n_z * ax - n_x * az;
        double na_z = n_x * ay - n_y * ax;
        double mod_na = std::hypot(na_x, na_y, na_z);

        e_x = {na_x / mod_na, na_y / mod_na, na_z / mod_na};

        const auto [ex_x, ex_y, ex_z] = e_x;
        e_y = {n_y * ex_z - n_z * ex_y, n_z * ex_x - n_x * ex_z, n_x * ex_y - n_y * ex_x};

        const auto [ey_x, ey_y, ey_z] = e_y;
        const auto [r0_x, r0_y, r0_z] = r0;

        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (condition(index)) {
                double pr1 = projection1[index];
                double pr2 = projection2[index];
                double pr3 = projection3[index];
                const auto [new_pr1, new_pr2, new_pr3] = SC_rectangle.new_coordinate(pr1, pr2, pr3);
                if (new_pr1 >= bound_x.min && new_pr1 <= bound_x.max && new_pr2 >= bound_y.min && new_pr2 <= bound_y.max &&
                    new_pr3 >= bound_z.min && new_pr3 <= bound_z.max) {
                    double new_x = ex_x * (x[index] - r0_x) + ex_y * (y[index] - r0_y) + ex_z * (z[index] - r0_z);
                    double new_y = ey_x * (x[index] - r0_x) + ey_y * (y[index] - r0_y) + ey_z * (z[index] - r0_z);
                    double new_z = n_x * (x[index] - r0_x) + n_y * (y[index] - r0_y) + n_z * (z[index] - r0_z);
                    double pr1_area = [xy = &XY, new_x, new_y, new_z]() -> double {  // проекция для
                                                                                     // x
                        if (xy->first == ParametrsList::X) {
                            return new_x;
                        } else if (xy->first == ParametrsList::Y) {
                            return new_y;
                        } else {
                            return new_z;
                        }
                    }();

                    double pr2_area = [xy = &XY, new_x, new_y, new_z]() -> double {  // проекция для
                                                                                     // y
                        if (xy->second == ParametrsList::X) {
                            return new_x;
                        } else if (xy->second == ParametrsList::Y) {
                            return new_y;
                        } else {
                            return new_z;
                        }
                    }();
                    if (pr1_area >= limits_x.min && pr1_area <= limits_x.max && pr2_area >= limits_y.min &&
                        pr2_area <= limits_y.max) {
                        int ib = static_cast<int>((pr1_area - limits_x.min) / hb);
                        int jb = static_cast<int>((pr2_area - limits_y.min) / hb);
                        double new_v_x = ex_x * vx[index] + ex_y * vy[index] + ex_z * vz[index];
                        double new_v_y = ey_x * vx[index] + ey_y * vy[index] + ey_z * vz[index];
                        double r = std::hypot(new_x, new_y);
                        double vfi = (r > 0.0) ? (new_x * new_v_x + new_y * new_v_y) / r : 0.0;
                        Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += vfi;
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
template <class Predicate>
void Converter::calculate_Vfi(std::vector<double>& projection1, std::vector<double>& projection2,
                              std::vector<double>& projection3, const Predicate& condition)
{
    const auto& moment_impulse = calculate_L(condition);  // вычисление полного момента импульса системы
    // число строк в каждом файле
    const auto& Ns = data.get_Ns();
    const auto& offests = data.get_offsets();
    const auto& x = data.get_x();
    const auto& y = data.get_y();
    const auto& z = data.get_z();
    const auto& vx = data.get_vx();
    const auto& vy = data.get_vy();
    const auto& vz = data.get_vz();
    std::vector<size_t> Nij_b;
    Nij_b.resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    //------------------------------------вычисления значений на сетке----------------------------------
    Z.back().resize(Nb_XY.Nx * Nb_XY.Ny * Nfiles_into_clomun);
    for (size_t i = 0; i < data.get_ibuff_size(); ++i) {
        size_t Z_offset = Nb_XY.Nx * Nb_XY.Ny * i;
        const auto& [Lx, Ly, Lz] = moment_impulse[i];

        constexpr double EPS = 1e-6;
        constexpr double Unit_Tol = 0.99;

        std::array<double, 3> r0 = calculate_Center_of_Mass(i, condition);  // новый центр координат

        std::array<double, 3> e_z;  // вектор нормали к плоскости диска
        double L = std::hypot(Lx, Ly, Lz);
        if (L > EPS) {
            const double div_L = 1.0 / L;
            e_z = {Lx * div_L, Ly * div_L, Lz * div_L};
        } else {
            e_z = {0, 0, 1};
        }

        std::array<double, 3> e_x, e_y;
        const auto [n_x, n_y, n_z] = e_z;

        std::array<double, 3> a;  // вспомогательный вектор для определения новой системы координат
        if (std::hypot(n_x, n_y, n_z) > Unit_Tol) {
            a = {1, 0, 0};
        } else {
            a = {0, 0, 1};
        }
        const auto [ax, ay, az] = a;
        double na_x = n_y * az - n_z * ay;
        double na_y = n_z * ax - n_x * az;
        double na_z = n_x * ay - n_y * ax;
        double mod_na = std::hypot(na_x, na_y, na_z);

        e_x = {na_x / mod_na, na_y / mod_na, na_z / mod_na};

        const auto [ex_x, ex_y, ex_z] = e_x;
        e_y = {n_y * ex_z - n_z * ex_y, n_z * ex_x - n_x * ex_z, n_x * ex_y - n_y * ex_x};

        const auto [ey_x, ey_y, ey_z] = e_y;
        const auto [r0_x, r0_y, r0_z] = r0;

        for (int j = 0; j < Ns[i]; ++j) {
            size_t index = offests[i] - offests[0] + j;
            if (condition(index)) {
                double pr1 = projection1[index];
                double pr2 = projection2[index];
                double pr3 = projection3[index];
                const auto [new_pr1, new_pr2, new_pr3] = SC_rectangle.new_coordinate(pr1, pr2, pr3);
                if (new_pr1 >= bound_x.min && new_pr1 <= bound_x.max && new_pr2 >= bound_y.min && new_pr2 <= bound_y.max &&
                    new_pr3 >= bound_z.min && new_pr3 <= bound_z.max) {
                    double new_x = ex_x * (x[index] - r0_x) + ex_y * (y[index] - r0_y) + ex_z * (z[index] - r0_z);
                    double new_y = ey_x * (x[index] - r0_x) + ey_y * (y[index] - r0_y) + ey_z * (z[index] - r0_z);
                    double new_z = n_x * (x[index] - r0_x) + n_y * (y[index] - r0_y) + n_z * (z[index] - r0_z);
                    double pr1_area = [xy = &XY, new_x, new_y, new_z]() -> double {  // проекция для
                                                                                     // x
                        if (xy->first == ParametrsList::X) {
                            return new_x;
                        } else if (xy->first == ParametrsList::Y) {
                            return new_y;
                        } else {
                            return new_z;
                        }
                    }();

                    double pr2_area = [xy = &XY, new_x, new_y, new_z]() -> double {  // проекция для
                                                                                     // y
                        if (xy->second == ParametrsList::X) {
                            return new_x;
                        } else if (xy->second == ParametrsList::Y) {
                            return new_y;
                        } else {
                            return new_z;
                        }
                    }();
                    if (pr1_area >= limits_x.min && pr1_area <= limits_x.max && pr2_area >= limits_y.min &&
                        pr2_area <= limits_y.max) {
                        int ib = static_cast<int>((pr1_area - limits_x.min) / hb);
                        int jb = static_cast<int>((pr2_area - limits_y.min) / hb);
                        double new_v_x = ex_x * vx[index] + ex_y * vy[index] + ex_z * vz[index];
                        double new_v_y = ey_x * vx[index] + ey_y * vy[index] + ey_z * vz[index];
                        double r = std::hypot(new_x, new_y);
                        double vfi = (r > 0.0) ? (new_x * new_v_y - new_y * new_v_x) / r : 0.0;  // возможно не
                                                                                                 // правильно для YZ
                        Z.back()[ib + jb * Nb_XY.Ny + Z_offset] += vfi;
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