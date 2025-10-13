#pragma once
#include "Converter.h"
class Converter_Stars : protected Converter
{
   protected:
    double dV;    //_hb2 / (zmax - zmin);
    double _hb3;  // 1/hb^3

   public:
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
