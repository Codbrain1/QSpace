#include <cerrno>
#include <cstddef>
#include <filesystem>
#include <vector>

#include "Converter/DataStorage.h"

std::vector<double>& DataStorage::get_x()
{
    if (!x.empty())
        return x;
    else
        throw std::runtime_error("x is not allocated");
}
std::vector<double>& DataStorage::get_y()
{
    if (!y.empty())
        return y;
    else
        throw std::runtime_error("y is not allocated");
}
std::vector<double>& DataStorage::get_z()
{
    if (!z.empty())
        return z;
    else
        throw std::runtime_error("z is not allocated");
}
std::vector<double>& DataStorage::get_vx()
{
    if (!vx.empty())
        return vx;
    else
        throw std::runtime_error("vx is not allocated");
}
std::vector<double>& DataStorage::get_vy()
{
    if (!vy.empty())
        return vy;
    else
        throw std::runtime_error("vy is not allocated");
}
std::vector<double>& DataStorage::get_vz()
{
    if (!vz.empty())
        return vz;
    else
        throw std::runtime_error("vz is not allocated");
}
std::vector<double>& DataStorage::get_rho()
{
    if (!rho.empty())
        return rho;
    else
        throw std::runtime_error("rho is not allocated");
}
std::vector<double>& DataStorage::get_e()
{
    if (!e.empty())
        return e;
    else
        throw std::runtime_error("e is not allocated");
}
std::vector<double>& DataStorage::get_m()
{
    if (!m.empty())
        return m;
    else
        throw std::runtime_error("m is not allocated");
}
std::vector<int>& DataStorage::get_ind_sph()
{
    if (!ind_sph.empty())
        return ind_sph;
    else
        throw std::runtime_error("ind_sph is not allocated");
}
std::vector<double>& DataStorage::get_t_MCYS()
{
    if (!t_MCYS.empty())
        return t_MCYS;
    else
        throw std::runtime_error("t_MCYS is not allocated");
}
std::vector<size_t>& DataStorage::get_Ns()
{
    if (!Ns.empty())
        return Ns;
    else
        throw std::runtime_error("Ns is not allocated");
}
std::vector<size_t>& DataStorage::get_offsets()
{
    if (!offsets.empty()) {
        return offsets;
    } else {
        throw std::runtime_error("Ns is not allocated");
    }
}
size_t DataStorage::get_count_files()
{
    return ifile_names.size();
}
size_t DataStorage::get_ibuff_size()
{
    return ibuff_size;
}
std::vector<std::filesystem::path> DataStorage::get_last_file_names()
{
    return std::vector<std::filesystem::path>(ifile_names.begin() + current_cursor - ibuff_size,
                                              ifile_names.begin() + current_cursor);
}