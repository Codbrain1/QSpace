#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "ConstantsParametrs.h"
#include "Converter/DataStorage.h"
void DataStorage::set_buff_size(const size_t size)
{
    if (size > 0) {
        ibuff_size = size;
        ifiles.clear();
        ifiles.reserve(ibuff_size);
        ifiles.assign(ibuff_size, nullptr);
    } else
        throw std::invalid_argument("buff size must not be less 0");
}
void DataStorage::set_file_buffer_size(const size_t size)
{
    if (size > 0) {
        ifile_buffer_size = size;
    } else
        throw std::invalid_argument("file buff size must not be less 0");
}
void DataStorage::reset_cursor() noexcept
{
    current_cursor = 0;
}
void DataStorage::set_cursor(const size_t cursor)
{
    if (cursor > 0) {
        current_cursor = cursor;
    } else
        throw std::invalid_argument("cursor must not be less 0");
}
void DataStorage::setup_columns(const std::vector<std::string>& columns)
{
    if (!columns.empty()) {
        column_list.reserve(columns.size());

        for (const auto& column : columns) {
            if (column == ParametrsList::X) {
                column_list.push_back(&x);  // x координата
            } else if (column == ParametrsList::Y) {
                column_list.push_back(&y);  // y координата
            } else if (column == ParametrsList::Z) {
                column_list.push_back(&z);  // z координата
            } else if (column == ParametrsList::VX) {
                column_list.push_back(&vx);  // vx компонента скорости
            } else if (column == ParametrsList::VY) {
                column_list.push_back(&vy);  // vy компонента скорости
            } else if (column == ParametrsList::VZ) {
                column_list.push_back(&vz);  // vz компонента скорости
            } else if (column == ParametrsList::Rho) {
                column_list.push_back(&rho);  // плотность
            } else if (column == ParametrsList::e) {
                column_list.push_back(&e);  // внутреняя энергия
            } else if (column == ParametrsList::m) {
                column_list.push_back(&m);  // масса
            } else if (column == ParametrsList::ind_sph) {
                column_list.push_back((&ind_sph));
            } else if (column == ParametrsList::t_MCYS) {
                column_list.push_back(&t_MCYS);
            } else {
                column_list.push_back(nullptr);
            }
        }
    } else
        throw std::invalid_argument("vector columns has empty value");
}