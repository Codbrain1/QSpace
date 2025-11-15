#pragma once
#include "IConverter_Hydrodinamics.h"
class Converter_YoungStars : public IConverter_Hydrodinamics
{
   protected:
    void calculate_LgT(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3);
    void calculate_T(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3);

    void calculate_LgRho(std::vector<double>& projection1, std::vector<double>& projection2,
                         std::vector<double>& projection3);
    void calculate_Rho(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3);

    void calculate_Vr(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3,
                      std::vector<double>& v_projection1, std::vector<double>& v_projection2);
    void calculate_Vfi(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3,
                       std::vector<double>& v_projection1, std::vector<double>& v_projection2);

   public:
    Converter_YoungStars(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy,
                         std::filesystem::path _output_path)
        : IConverter_Hydrodinamics(_data, c, _Nbxy, _output_path) {};
    void convert();  // функция конвертации
    // lg означает десятичный логарифм
};