#pragma once
#include "Converter.h"
class Converter_Stars : public Converter
{
   protected:
    double _hb3;  // 1/hb^3

   public:
    Converter_Stars(DataStorage& _data, ParametrsList::iniConstants& c, count_cell _Nbxy);
    void convert();  // функция конвертации
    // lg означает десятичный логарифм
    void calculate_LgSigma();
    void calculate_Sigma();
    void calculate_LgRho();
    void calculate_Rho();
    void calculate_Vr();
    void calculate_Vfi();
    void calculate_Vx();
    void calculate_Vy();
    void calculate_Vz();
};
