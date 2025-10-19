#pragma once
#include "Converter.h"
class Converter_YoungStars : public Converter
{
   protected:
    double gamma;   // адиабатическая постоянная
    double gamma1;  //=gamma-1.0
    double l_Te;    // l_Te = gamma * gamma1 * 10000 / (100.0 / (l_v * l_v));  --> K

   public:
    Converter_YoungStars(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy);
    void convert();  // функция конвертации
    // lg означает десятичный логарифм
    void calculate_LgSigma();
    void calculate_Sigma();
    void calculate_LgRho();
    void calculate_LgT();
    void calculate_T();
    void calculate_Rho();
    void calculate_Vr();
    void calculate_Vfi();
    void calculate_Vx();
    void calculate_Vy();
    void calculate_Vz();
};