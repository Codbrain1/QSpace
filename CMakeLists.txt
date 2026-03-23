cmake_minimum_required(VERSION 3.16) #минимальная версия cmake

# 1. основные сведедения о проекте
#-------------------------------------------------------------------
project(QSpace VERSION 0.6.1 LANGUAGES CXX) # название проекта его версия и язык на котором написан проект

set(CMAKE_CXX_STANDARD 20) # стандарт языка 23
set(CMAKE_CXX_STANDARD_REQUIRED ON)#обязательный стандарт языка
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(ENABLE_TESTS "Build Tests" ON) #опция переключения между тестами

# 2. Настройка окружения проекта (Compiler)

#=========== Настройки для виндовс ==============
if(WIN32) 
    set(CMAKE_PREFIX_PATH 
    "C:/Qt/6.9.2/mingw_64"
    "D:/NIR/NIR_6_semestr/vtk/vtk-install-ffmpeg-wmf-dll/lib/cmake/vtk-9.5")
    # Отключение Vulkan
    set(QT_NO_VULKAN ON)
    set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe")
    set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1310_64/bin/gcc.exe")
endif()

# 3. поиск зависимостей
# Core - базовые классы
# Gui - графика
# Widgets - различные виджеты(окна, кнопки и т.д.)
# LinguistTools - модуль с переводами
find_package(Qt6 REQUIRED COMPONENTS 
            Core 
            Gui 
            Widgets 
            LinguistTools)
find_package(VTK REQUIRED COMPONENTS 
            CommonCore 
            CommonDataModel 
            CommonColor
            IOCore
            IOOggTheora
            IOVideo
            GUISupportQt
            )

# автоматическая генерация для qt
#------------------------------------
set(CMAKE_AUTOMOC ON) # создает moc_*.cpp из .h c Q_Object
set(CMAKE_AUTORCC ON) #встраивает qrc ресурсы
set(CMAKE_AUTOUIC ON) #обработка .ui файлов
set(QT_NO_QML_PLUGINS ON) #отключаем qml плагины
set(QT6_NO_IMPORT_PLUGINS ON)
set(QT_NO_VIRTUALKEYBOARD ON)
#------------------------------------

# 4. флаги компиляции 
#--------------------------------------
add_library(project_config INTERFACE)
target_compile_options(project_config INTERFACE -Wall -Wextra -fopenmp)
target_compile_options(project_config INTERFACE $<$<CONFIG:Release>:-O2>)
target_compile_options(project_config INTERFACE $<$<CONFIG:Debug>: -O0 -g3 -fno-omit-frame-pointer>)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# 5. исполняемые файлы помещают в bin, библиотеки в lib
#------------------------------------------
set(OUTPUT_SUBDIR $<CONFIG>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_SUBDIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_SUBDIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${OUTPUT_SUBDIR}/lib)

if(ENABLE_TESTS)
    message("-- Test Enabled")
    enable_testing()
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Tests.cmake)
endif()

add_subdirectory(app/QSpaceDesktop)
add_subdirectory(Common)
add_subdirectory(Core)
# add_subdirectory(Filtering)
add_subdirectory(IO)
# add_subdirectory(Physics)
# add_subdirectory(Remote)
add_subdirectory(Visualize)
add_subdirectory(Session)
add_subdirectory(UI)
# add_subdirectory(Charting)

include(GNUInstallDirs)
set(CPACK_PACKAGE_NAME "QSpace")
set(CPACK_PACKAGE_VENDOR "Shorkin")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_GENERATOR "NSIS")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "QSpace")
set(CPACK_NSIS_DISPLAY_NAME "QSpace Visualization Tool")
include(CPack)
