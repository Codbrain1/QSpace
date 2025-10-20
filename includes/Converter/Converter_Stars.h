#pragma once
#include "Converter.h"
class Converter_Stars : public Converter
{
   protected:
    double _hb3;  // 1/hb^3
    void calculate_LgSigma(std::vector<double>& projection1, std::vector<double>& projection2);
    void calculate_Sigma(std::vector<double>& projection1, std::vector<double>& projection2);
    void calculate_LgRho(std::vector<double>& projection1, std::vector<double>& projection2,
                         std::vector<double>& projection3);
    void calculate_Rho(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3);

    void calculate_Vr(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3,
                      std::vector<double>& v_projection1, std::vector<double>& v_projection2);
    void calculate_Vfi(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3,
                       std::vector<double>& v_projection1, std::vector<double>& v_projection2);
    void calculate_V_projection(std::vector<double>& projection1, std::vector<double>& projection2,
                                std::vector<double>& projection3, std::vector<double>& v_projection);

   public:
    Converter_Stars(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy, std::filesystem::path _output_path);
    void convert();  // функция конвертации
    // lg означает десятичный логарифм
};
