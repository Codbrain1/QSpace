#pragma once
#include <filesystem>

#include "IConverter_NbodyOnly.h"
class Converter_DarkMatter : public IConverter_NbodyOnly
{
   public:
    Converter_DarkMatter(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy,
                         std::filesystem::path _output_path)
        : IConverter_NbodyOnly(_data, c, _Nbxy, _output_path) {};
    void convert();  // функция конвертации
    // lg означает десятичный логарифм
};
