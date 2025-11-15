#pragma once
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>
template <class T>
    requires std::is_arithmetic_v<T>
struct lim {
   public:
    T max;
    T min;
    void set_type_limits()
    {
        max = std::numeric_limits<T>::min();
        min = std::numeric_limits<T>::max();
    }
    lim() { set_type_limits(); }
    lim(T _max, T _min) : max(_max), min(_min) {}
};
struct count_cell {
   public:
    short Nx, Ny;
    count_cell() : Nx(0), Ny(0) {}
    count_cell(int _Nx, int _Ny) : Nx(_Nx), Ny(_Ny) {}
};
struct SistemCoordinate_Rectangle {
    double cx, cy, cz;
    double r00, r01, r02, r10, r11, r12, r20, r21, r22;
    SistemCoordinate_Rectangle()
        : cx(0), cy(0), cz(0), r00(0), r01(0), r02(0), r10(0), r11(0), r12(0), r20(0), r21(0), r22(0)
    {
    }

    void setup_SC(lim<double> xlim, lim<double> ylim, lim<double> zlim, double alpha, double beta, double phi)
    {
        cx = 0.5 * (xlim.max + xlim.min), cy = 0.5 * (ylim.max + ylim.min), cz = 0.5 * (zlim.max + zlim.min);
        alpha *= M_PI / 180.0;
        beta *= M_PI / 180.0;
        phi *= M_PI / 180.0;
        r00 = std::cos(phi) * std::cos(beta);
        r01 = std::cos(phi) * std::sin(beta) * std::sin(alpha) - std::sin(phi) * std::cos(alpha);
        r02 = std::cos(phi) * std::sin(beta) * std::cos(alpha) + std::sin(phi) * std::sin(alpha);
        r10 = std::sin(phi) * std::cos(beta);
        r11 = std::sin(phi) * std::sin(beta) * std::sin(alpha) + std::cos(phi) * std::cos(alpha);
        r12 = std::cos(phi) * std::sin(beta) * std::cos(alpha) - std::cos(phi) * std::sin(alpha);
        r20 = -std::sin(beta);
        r21 = std::cos(beta) * std::sin(alpha);
        r22 = std::cos(beta) * std::cos(alpha);
    }
    inline std::array<double, 3> new_coordinate(double x, double y, double z)
    {
        double newx = r00 * (x - cx) + r10 * (y - cy) + r20 * (z - cz);
        double newy = r01 * (x - cx) + r11 * (y - cy) + r21 * (z - cz);
        double newz = r02 * (x - cx) + r12 * (y - cy) + r22 * (z - cz);
        return {newx, newy, newz};
    }
};
struct SistemCoordinate_Area {
    double r0, r1, r2, r3, r4;
    double op1, op2;
    SistemCoordinate_Area() : r0(0), r1(0), r2(0), op1(0), op2(0) {}
    void setup_SC(lim<double> x, lim<double> y, double teta, double psi)
    {
        op1 = 0.5 * (x.min + x.max);
        op2 = 0.5 * (y.min + y.max);
        psi *= M_PI / 180.0;
        teta *= M_PI / 180.0;
        double cos_psi = std::cos(psi);
        double cos_teta = std::cos(teta);
        double sin_psi = std::sin(psi);
        double sin_teta = std::sin(teta);
        r0 = cos_psi;
        r1 = sin_teta * sin_psi;
        r2 = cos_teta * sin_psi;
        r3 = cos_teta;
        r4 = -sin_teta;
    }
    inline std::array<double, 2> new_coordinate(double x, double y, double z)
    {
        double new_pr1 = (x - op1) * r0 + (y - op2) * r1 + z * r2;
        double new_pr2 = (y - op2) * r3 + z * r4;
        return {new_pr1, new_pr2};
    }
};