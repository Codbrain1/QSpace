#pragma once
#include "IConverter_NbodyOnly.h"
class Converter_Stars : public IConverter_NbodyOnly
{
   public:
    Converter_Stars(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy, std::filesystem::path _output_path)
        : IConverter_NbodyOnly(_data, c, _Nbxy, _output_path) {};
    void convert();  // функция конвертации
    // lg означает десятичный логарифм
};
