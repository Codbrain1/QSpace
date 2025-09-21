cmake_minimum_required(VERSION 3.16)

project(TxtToGrdConverterTests)
add_executable(Test_Converter Converter_FUNCTIONAL_TEST.cpp ${CMAKE_SOURCE_DIR}/includes/Converter/Converter.h)

set_target_properties(Test_Converter PROPERTIES
AUTOMOC OFF
AUTOUIC OFF
AUTORCC OFF
)

add_test(NAME FUNC_TEST_Converter COMMAND Test_Converter)

target_include_directories(Test_Converter PRIVATE ${CMAKE_SOURCE_DIR}/includes)

