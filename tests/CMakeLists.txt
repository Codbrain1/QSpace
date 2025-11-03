#минимальная версия cmake
cmake_minimum_required(VERSION 3.16)

#все исходные тестируемые файлы компилируются в статическую библиотеку а после используются
add_library(test_common STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Converter/DataStorage/DataStorage_file_func.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Converter/DataStorage/DataStorage_gets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Converter/DataStorage/DataStorage_sets.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Converter/Converter/Converter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Converter/Converter/Converter_DarkMatter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Converter/Converter/Converter_Gas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Converter/Converter/Converter_MolecularClouds.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Converter/Converter/Converter_Stars.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/Converter/Converter/Converter_YoungStars.cpp
)   

#все кто линкуется видят общие заголовки
target_include_directories(test_common PUBLIC
${CMAKE_CURRENT_SOURCE_DIR}/../includes
)

#требование 23-го стандарта с++
target_compile_features(test_common PUBLIC cxx_std_23)

#генератор выражения: если debug то флаги для отладки, если release то флаги для release
target_compile_options(test_common PRIVATE
    $<$<CONFIG:Debug>: -Wall -Wextra -Wpedantic -g -O0 -fno-omit-frame-pointer>
    $<$<CONFIG:Release>: -Wall -Wextra -Wpedantic -O2>
)

#санитайзеры только для отладки
target_link_options(test_common PRIVATE
    $<$<CONFIG:Debug>: -fsanitize=address -fsanitize=undefined>
)

# === Функция для создания теста ===
function(add_unit_test name test_source)
    add_executable(${name} ${test_source})
    target_link_libraries(${name} PRIVATE test_common)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../includes)
    target_compile_features(${name} PRIVATE cxx_std_23)

    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES LABELS "UnitTest")
endfunction()

# === Добавляем тесты ===
add_unit_test(test_datastorage test_datastorage.cpp)
add_unit_test(test_converter_darkmatter test_converter_darkmatter.cpp)
add_unit_test(test_converter_gas test_converter_gas.cpp)
add_unit_test(test_converter_molecularclouds test_converter_molecularclouds.cpp)
add_unit_test(test_converter_stars test_converter_stars.cpp)
add_unit_test(test_converter_youngstars test_converter_youngstars.cpp)

# Явные метки
set_tests_properties(test_datastorage PROPERTIES LABELS "DataStorage")
set_tests_properties(test_converter_darkmatter PROPERTIES LABELS "Converter_DarkMatter")
set_tests_properties(test_converter_gas PROPERTIES LABELS "Converter_Gas")
set_tests_properties(test_converter_molecularclouds PROPERTIES LABELS "Converter_MolecularClouds")
set_tests_properties(test_converter_stars PROPERTIES LABELS "Converter_Stars")
set_tests_properties(test_converter_youngstars PROPERTIES LABELS "Converter_YoungStars")
