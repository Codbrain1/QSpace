#pragma once
#include <array>
#include <string_view>
class ParametrsList
{
   public:
    // список параметров типов выходных данных
    static constexpr std::array<std::string_view, 5> Type_output_data{"Dark matter", "Gas", "Molecular clouds", "Stars",
                                                                      "Young stars"};
    // список параметров типа выходных данных без температуры
    static constexpr std::array<std::string_view, 2> NoTermal_odata_t{Type_output_data[0], Type_output_data[3]};
    // список параметров типа выходных данных с температурой
    static constexpr std::array<std::string_view, 3> Termal_odata_t{Type_output_data[1], Type_output_data[2],
                                                                    Type_output_data[4]};
    //-----------------------------------------------------------------------
    // доступ к типам выходных данных по названиям
    static constexpr std::string_view DarkMatter_type = Type_output_data[0];
    static constexpr std::string_view Gas_type = Type_output_data[1];
    static constexpr std::string_view MolecularClouds_type = Type_output_data[2];
    static constexpr std::string_view Stars_type = Type_output_data[3];
    static constexpr std::string_view YongStars_type = Type_output_data[4];

    //-----------------------------------------------------------------------
    // список разрешенных выходных параметров
    static constexpr std::array<std::string_view, 13> Z_outParams = {"R (объемная плотность)",
                                                                     "LgR (десятичный логарифм от объемной плотности)",
                                                                     "LgS (десятичный логарифм от поверхностной плотности)",
                                                                     "S (поверхностная плотность)",
                                                                     "Vfi (азимутальная скорость)",
                                                                     "Vr (радиальная скорость)",
                                                                     "Vx (X-компонента скорости)",
                                                                     "Vy (Y-компонента скорости)",
                                                                     "Vz (Z-компонента скорости)",
                                                                     "|V| (модуль скорости)",
                                                                     "c_r (X-компонента дисперсии скорости)",
                                                                     "c_phi (X-компонента дисперсии скорости)",
                                                                     "c_z (X-компонента дисперсии скорости)"};
    static constexpr std::string_view Z_outParams_Rho = Z_outParams[0];
    static constexpr std::string_view Z_outParams_LgRho = Z_outParams[1];
    static constexpr std::string_view Z_outParams_LgSigma = Z_outParams[2];
    static constexpr std::string_view Z_outParams_Sigma = Z_outParams[3];
    static constexpr std::string_view Z_outParams_Vfi = Z_outParams[4];
    static constexpr std::string_view Z_outParams_Vr = Z_outParams[5];
    static constexpr std::string_view Z_outParams_Vx = Z_outParams[6];
    static constexpr std::string_view Z_outParams_Vy = Z_outParams[7];
    static constexpr std::string_view Z_outParams_Vz = Z_outParams[8];
    static constexpr std::string_view Z_outParams_V = Z_outParams[9];
    static constexpr std::string_view Z_outParams_c_r = Z_outParams[10];
    static constexpr std::string_view Z_outParams_c_phi = Z_outParams[11];
    static constexpr std::string_view Z_outParams_c_z = Z_outParams[12];
    static constexpr std::string_view Z_outParamT = "T (температура)";
    static constexpr std::string_view Z_outParamLgT = "LgT (десятичный логарифм от температуры)";
    static constexpr std::string_view Z_outParamKit = "Набор параметров";

    //-----------------------------------------------------------------------
    // столбцы в файле
    static constexpr std::array<std::string_view, 11> Columns_names{"x (координата)",
                                                                    "y (координата)",
                                                                    "z (координата)",
                                                                    "rho (плотность)",
                                                                    "vx (x компонента скорости)",
                                                                    "vy (y компонента скорости)",
                                                                    "vz (z компонента скорости)",
                                                                    "e (внутреняя энергия)",
                                                                    "m (масса частицы)",
                                                                    "ind_sph",
                                                                    "t_MCYS"};

    static constexpr std::string_view X = Columns_names[0];
    static constexpr std::string_view Y = Columns_names[1];
    static constexpr std::string_view Z = Columns_names[2];
    static constexpr std::string_view Rho = Columns_names[3];
    static constexpr std::string_view VX = Columns_names[4];
    static constexpr std::string_view VY = Columns_names[5];
    static constexpr std::string_view VZ = Columns_names[6];
    static constexpr std::string_view e = Columns_names[7];
    static constexpr std::string_view m = Columns_names[8];
    static constexpr std::string_view ind_sph = Columns_names[9];
    static constexpr std::string_view t_MCYS = Columns_names[10];

    static constexpr std::array<std::string_view, 7> Columns_names_DM_S{X, Y, Z, VX, VY, VZ, m};
    struct iniConstants {
        double gamma, Km, Kr;
        double hb;
    };
    static constexpr std::string_view is_bin_grd = ".grd (бинарный)";
    static constexpr std::string_view is_txt_grd = ".grd (текстовый)";
    static constexpr std::string_view is_bin_ifiles = "бинарный";
    static constexpr std::string_view is_txt_ifiles = "текстовый";
    ParametrsList() = delete;
};