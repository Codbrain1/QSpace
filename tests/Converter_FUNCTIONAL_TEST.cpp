#include <cstdlib>
#include <initializer_list>
#include <iostream>

#include "Converter/Converter.h"
int main()
{
    try {
        DataStorage storage({"Datasets/G_00010.txt"});

        storage.setup_columns({std::string(ParametrsList::X)});
        storage.load_file_metadate_txt();

    } catch (const char* ex) {
        std::cerr << ex;
    }
}