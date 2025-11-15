#pragma once
#include "Converter.h"
class IConverter_NbodyOnly : public Converter
{
   protected:
    double _hb3;  // 1/hb^3
    void calculate_LgSigma(std::vector<double>& projection1, std::vector<double>& projection2,
                           std::vector<double>& projection3);
    void calculate_LgRho(std::vector<double>& projection1, std::vector<double>& projection2,
                         std::vector<double>& projection3);
    void calculate_Rho(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3);

   public:
    IConverter_NbodyOnly(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy,
                         std::filesystem::path _output_path)
        : Converter(_data, c, _Nbxy, _output_path)
    {
        _hb3 = _hb2 / hb;
    }
};