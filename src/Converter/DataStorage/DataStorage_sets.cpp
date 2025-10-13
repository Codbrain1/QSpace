#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "Converter/DataStorage.h"
void DataStorage::set_buff_size(const size_t size)
{
    if (size > 0) {
        buff_size = size;
        ifiles.clear();
        ifiles.reserve(buff_size);
        ifiles.assign(buff_size, nullptr);
    }
    throw std::invalid_argument("buff size must not be less 0");
}
void DataStorage::set_file_buffer_size(const size_t size)
{
    if (size > 0) {
        file_buffer_size = size;
    }
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
    }
    throw std::invalid_argument("cursor must not be less 0");
}
void DataStorage::setup_columns(const std::vector<int>& columns)
{
    if (!columns.empty()) {
        column_list.reserve(columns.size());

        for (const auto& column : columns) {
            switch (column) {
                case 0:
                    column_list.push_back(&x);  // x координата

                    break;
                case 1:
                    column_list.push_back(&y);  // y координата

                    break;
                case 2:
                    column_list.push_back(&z);  // z координата

                    break;
                case 3:
                    column_list.push_back(&vx);  // vx компонента скорости

                    break;
                case 4:
                    column_list.push_back(&vy);  // vy компонента скорости

                    break;
                case 5:
                    column_list.push_back(&vz);  // vz компонента скорости

                    break;
                case 6:
                    column_list.push_back(&rho);  // плотность

                    break;
                case 7:
                    column_list.push_back(&e);  // внутреняя энергия

                    break;
                case 8:
                    column_list.push_back(&m);  // масса
                    break;
                case 9:
                    column_list.push_back((&ind_sph));

                    break;
                case 10:
                    column_list.push_back(&t_MCYS);

                    break;
                default:
                    column_list.push_back(nullptr);
            }
        }
    } else
        throw std::invalid_argument("vector columns has empty value");
}