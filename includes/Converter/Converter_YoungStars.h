#pragma once
#include "Converter.h"
class Converter_YoungStars : public Converter
{
   protected:
    double gamma;   // адиабатическая постоянная
    double gamma1;  //=gamma-1.0
    double l_Te;    // l_Te = gamma * gamma1 * 10000 / (100.0 / (l_v * l_v));  --> K
    void calculate_LgT(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3);
    void calculate_T(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3);
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
    Converter_YoungStars(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy,
                         std::filesystem::path _output_path);
    void convert();  // функция конвертации
    // lg означает десятичный логарифм
};