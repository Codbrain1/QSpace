cmake_minimum_required(VERSION 3.16) #минимальная версия cmake

project(QSpace VERSION 0.3 LANGUAGES CXX) # название проекта его версия и язык на котором написан проект
option(ENABLE_TESTS "Build Convert library tests" OFF) #опция переключения между тестами

set(CMAKE_CXX_STANDARD 23) # стандарт языка 23
set(CMAKE_CXX_STANDARD_REQUIRED ON)#обязательный стандарт языка

#автоматическая генерация для qt
#------------------------------------
set(CMAKE_AUTOMOC ON) # создает moc_*.cpp из .h c Q_Object
set(CMAKE_AUTORCC ON) #встраивает qrc ресурсы
set(CMAKE_AUTOUIC ON) #обработка .ui файлов
#------------------------------------

#=========== Настройки для виндовс ==============
if(WIN32) 
    # Путь к статическому Qt6
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt-6.9.2-ninja")
    # Отключение Vulkan
    set(QT_NO_VULKAN ON)
    set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe")
    set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1310_64/bin/gcc.exe")
    # Флаги компиляции для статической сборки
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc")
endif()


#общие флаги компиляции
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall") #строгие предупреждения
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fopenmp") # для отладки без оптимизации
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -flto -fopenmp") # для запуска с оптимизацией O2 и оптимизация линковки

# выбор папки сборки в зависимости от типа сборки
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OUTPUT_SUBDIR "Debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(OUTPUT_SUBDIR "Release")
elseif(CMAKE_BUILD_TYPE STREQUAL "Test")  # Для тестов, если зададите -DCMAKE_BUILD_TYPE=Test
    set(OUTPUT_SUBDIR "Test")
else()
    set(OUTPUT_SUBDIR "default")  # Fallback
endif()

#исполняемые файлы помещают в bin, библиотеки в lib
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_SUBDIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_SUBDIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_SUBDIR}/lib)


# Поиск Qt6, включая LinguistTools
# Core - базовые классы
# Gui - графика
# Widgets - различные виджеты(окна, кнопки и т.д.)
# LinguistTools - модуль с переводами
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools OpenGL OpenGLWidgets)
# 2. ТЕПЕРЬ — отключаем QML-плагины
set(QT_NO_QML_PLUGINS ON)
set(QT6_NO_IMPORT_PLUGINS ON)
set(QT_NO_VIRTUALKEYBOARD ON)

# 3. Отключаем ненужные фичи
set(QT_FEATURE_multimedia OFF CACHE BOOL "Disable multimedia" FORCE)
set(QT_FEATURE_texttospeech OFF CACHE BOOL "Disable text-to-speech" FORCE)
set(QT_FEATURE_qt3d OFF CACHE BOOL "Disable Qt3D" FORCE)
# Файлы переводов
set(TS_FILES
    translates/TxtToGrdConvert_ru.ts
)
#добавление файлов перевода
qt6_add_translations(
    QSpace
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES
)
#добавляем таргет библиотеки
# add_library(Convert includes/Converter/Converter.h)

#списиок всех исходных файлов
set(PROJECT_SOURCES
src/main.cpp
src/UI/mainwindow.cpp
includes/mainwindow.h
src/UI/mainwindow_slots.cpp
src/UI/mainwindow_private_methods.cpp
includes/ConstantsParametrs.h
includes/Converter/DataStorage.h
includes/Converter/Converter.h
includes/Converter/Converter_DarkMatter.h
includes/Converter/Converter_Gas.h
includes/Converter/Converter_MolecularClouds.h
includes/Converter/Converter_Stars.h
includes/Converter/Converter_YoungStars.h
includes/Converter/IConverter_Hydrodinamics.h
includes/Converter/IConverter_NbodyOnly.h
src/Converter/Converter/Converter.cpp
src/Converter/Converter/IConverter_NbodyOnly.cpp
src/Converter/Converter/Converter_DarkMatter.cpp
src/Converter/Converter/Converter_Gas.cpp
src/Converter/Converter/Converter_MolecularClouds.cpp
src/Converter/Converter/Converter_Stars.cpp
src/Converter/Converter/Converter_YoungStars.cpp
src/Converter/DataStorage/DataStorage_file_func.cpp
src/Converter/DataStorage/DataStorage_gets.cpp
src/Converter/DataStorage/DataStorage_sets.cpp
src/UI/mainwindow.ui
${QM_FILES}
)

#создание исполнимого qt файла
qt_add_executable(QSpace
MANUAL_FINALIZATION
${PROJECT_SOURCES}
)

target_include_directories(QSpace PRIVATE ${CMAKE_SOURCE_DIR}/includes) #добавление путей к заголовкам
target_link_libraries(QSpace PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL
    Qt6::OpenGLWidgets) #линковка с qt

if(WIN32) # на виндовс только gui
set_target_properties(QSpace PROPERTIES
    WIN32_EXECUTABLE TRUE
)
endif()

include(GNUInstallDirs)
install(TARGETS QSpace
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(QSpace)

if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
