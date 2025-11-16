#pragma once
#include "IConverter_Hydrodinamics.h"
class Converter_YoungStars : public IConverter_Hydrodinamics
{
   public:
    Converter_YoungStars(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy,
                         std::filesystem::path _output_path)
        : IConverter_Hydrodinamics(_data, c, _Nbxy, _output_path) {};
    void convert();  // функция конвертации
    // lg означает десятичный логарифм
};