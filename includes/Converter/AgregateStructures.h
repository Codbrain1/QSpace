#pragma once
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