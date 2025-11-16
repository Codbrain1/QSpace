#pragma once
#include <array>
#include <vector>

#include "Converter.h"
class IConverter_Hydrodinamics : public Converter
{
   protected:
    double gamma;   // адиабатическая постоянная
    double gamma1;  //=gamma-1.0
    double l_Te;    // l_Te = gamma * gamma1 * 10000 / (100.0 / (l_v * l_v));  --> K

    template <class Predicate>
    void calculate_LgSigma(std::vector<double>& projection1, std::vector<double>& projection2,
                           std::vector<double>& projection3, const Predicate& condition);
    template <class Predicate>
    void calculate_LgRho(std::vector<double>& projection1, std::vector<double>& projection2,
                         std::vector<double>& projection3, const Predicate& condiditon);
    template <class Predicate>
    void calculate_Rho(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3,
                       const Predicate& condiditon);
    template <class Predicate>
    void calculate_T(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3,
                     const Predicate& condiditon);
    template <class Predicate>
    void calculate_LgT(std::vector<double>& projection1, std::vector<double>& projection2, std::vector<double>& projection3,
                       const Predicate& condiditon);

   public:
    IConverter_Hydrodinamics(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy,
                             std::filesystem::path _output_path)
        : Converter(_data, c, _Nbxy, _output_path)
    {
        gamma = c.gamma;
        gamma1 = c.gamma - 1.0;
        l_Te = gamma * gamma1 * 10000 / (100.0 / (l_v * l_v));
    }
};
#include "IConverter_Hydrodinamics.tpp"