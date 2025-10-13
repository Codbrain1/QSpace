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
    static constexpr std::array<std::string_view, 9> Z_outParams = {"R (объемная плотность)",
                                                                    "LgR (десятичный логарифм от объемной плотности)",
                                                                    "LgS (десятичный логарифм от поверхностной плотности)",
                                                                    "S (поверхностная плотность)",
                                                                    "Vfi (азимутальная скорость)",
                                                                    "Vr (радиальная скорость)",
                                                                    "Vx (X-компонента скорости)",
                                                                    "Vy (Y-компонента скорости)",
                                                                    "Vz (Z-компонента скорости)"};
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
};