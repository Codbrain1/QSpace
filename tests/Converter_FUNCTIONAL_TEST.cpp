#include <initializer_list>
#include <iostream>

#include "Converter/Converter.h"
int main()
{
    Translator<double> ts;
    try {
        ts.load_input_file("/home/mask/windows/научка/НИР_5_семестр/TxtToGrdConverter/tests/Datasets/G_00010.txt");
        ts.load_output_file("Output_G_00010.grd");
        ts.setup_columns(
            std::initializer_list<std::string>{"x", "y", "z", "density", "None", "None", "None", "None", "None", "None"},
            {"x", "y", "density"});
        ts.setup_gridXYZ({-3, 3}, {-3, 3}, {0, 0}, {100, 100});
        ts.read_file();
        ts.translate_to_grd_bindings();
    } catch (const char* ex) {
        std::cerr << ex;
    }
}