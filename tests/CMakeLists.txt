#минимальная версия cmake
cmake_minimum_required(VERSION 3.16)

set(TEST_COMPILE_FLAGS_DEBUG -Wall;-Wextra;-Wpedantic;-g;-O0;-fno-omit-frame-pointer)
set(TEST_COMPILE_FLAGS_RELEASE "-Wall -Wextra -Wpedantic -O2")
set(TEST_LINK_FLAGS_DEBUG -fsanitize=address;-fsanitize=undefined)
#тест для хранилища данных
add_executable(test_datastorage
test_datastorage.cpp
${CONVERTER_SOURCES})
target_include_directories(test_datastorage PRIVATE ${CMAKE_SOURCE_DIR}/includes)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test_datastorage PRIVATE ${TEST_COMPILE_FLAGS_DEBUG})
    target_link_options(test_datastorage PRIVATE ${TEST_LINK_FLAGS_DEBUG})
else()
    target_compile_options(test_datastorage PRIVATE ${TEST_COMPILE_FLAGS_RELEASE})
endif()

#тест для конвертера темной материи
add_executable(test_converter_darkmatter
test_converter_darkmatter.cpp
${CONVERTER_SOURCES})
target_include_directories(test_converter_darkmatter PRIVATE ${CMAKE_SOURCE_DIR}/includes)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test_converter_darkmatter PRIVATE ${TEST_COMPILE_FLAGS_DEBUG})
    target_link_options(test_converter_darkmatter PRIVATE ${TEST_LINK_FLAGS_DEBUG})
else()
    target_compile_options(test_converter_darkmatter PRIVATE ${TEST_COMPILE_FLAGS_RELEASE})
endif()

#тест для конвертера газа
add_executable(test_converter_gas
test_converter_gas.cpp
${CONVERTER_SOURCES})
target_include_directories(test_converter_gas PRIVATE ${CMAKE_SOURCE_DIR}/includes)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test_converter_gas PRIVATE ${TEST_COMPILE_FLAGS_DEBUG})
    target_link_options(test_converter_gas PRIVATE ${TEST_LINK_FLAGS_DEBUG})
else()
    target_compile_options(test_converter_gas PRIVATE ${TEST_COMPILE_FLAGS_RELEASE})
endif()

#тест для конвертера молекулярных облаков
add_executable(test_converter_molecularclouds
test_converter_molecularclouds.cpp
${CONVERTER_SOURCES})
target_include_directories(test_converter_molecularclouds PRIVATE ${CMAKE_SOURCE_DIR}/includes)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test_converter_molecularclouds PRIVATE ${TEST_COMPILE_FLAGS_DEBUG})
    target_link_options(test_converter_molecularclouds PRIVATE ${TEST_LINK_FLAGS_DEBUG})
else()
    target_compile_options(test_converter_molecularclouds PRIVATE ${TEST_COMPILE_FLAGS_RELEASE})
endif()

#тест для конвертера звезд
add_executable(test_converter_stars
test_converter_stars.cpp
${CONVERTER_SOURCES})
target_include_directories(test_converter_stars PRIVATE ${CMAKE_SOURCE_DIR}/includes)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test_converter_stars PRIVATE ${TEST_COMPILE_FLAGS_DEBUG})
    target_link_options(test_converter_stars PRIVATE ${TEST_LINK_FLAGS_DEBUG})
else()
    target_compile_options(test_converter_stars PRIVATE ${TEST_COMPILE_FLAGS_RELEASE})
endif()

# тест для конвертера молодых звезд
add_executable(test_converter_youngstars
test_converter_youngstars.cpp
${CONVERTER_SOURCES})
target_include_directories(test_converter_youngstars PRIVATE ${CMAKE_SOURCE_DIR}/includes)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(test_converter_youngstars PRIVATE ${TEST_COMPILE_FLAGS_DEBUG})
    target_link_options(test_converter_youngstars PRIVATE ${TEST_LINK_FLAGS_DEBUG})
else()
    target_compile_options(test_converter_youngstars PRIVATE ${TEST_COMPILE_FLAGS_RELEASE})
endif()

#добавление тестов в CTest
add_test(NAME DataStorageTests COMMAND test_datastorage)
add_test(NAME Converter_DarkMatterTests COMMAND test_converter_darkmatter)
add_test(NAME Converter_GasTests COMMAND test_converter_gas)
add_test(NAME Converter_MolecularCloudsTests COMMAND test_converter_molecularclouds)
add_test(NAME Converter_StarsTests COMMAND test_converter_stars)
add_test(NAME Converter_YoungStarsTests COMMAND test_converter_youngstars)

#групируем тесты по категориям
set_tests_properties(DataStorageTests PROPERTIES LABELS "DataStorage")
set_tests_properties(Converter_DarkMatterTests PROPERTIES LABELS "Converter")
set_tests_properties(Converter_GasTests PROPERTIES LABELS "Converter")
set_tests_properties(Converter_MolecularCloudsTests PROPERTIES LABELS "Converter")
set_tests_properties(Converter_StarsTests PROPERTIES LABELS "Converter")
set_tests_properties(Converter_YoungStarsTests PROPERTIES LABELS "Converter")

set_target_properties(
    test_datastorage
    test_converter_darkmatter
    test_converter_gas
    test_converter_molecularclouds
    test_converter_stars
    test_converter_youngstars
    PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

