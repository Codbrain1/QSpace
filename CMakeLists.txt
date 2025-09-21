cmake_minimum_required(VERSION 3.16)

project(TxtToGrdConvert VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(WIN32)
    # Путь к статическому Qt6
    set(CMAKE_PREFIX_PATH "C:/Qt/Qt-6.9.2")
    # Отключение Vulkan
    set(QT_NO_VULKAN ON)
    # Флаги компиляции для статической сборки
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -flto")

#устанавливаем директорию сборки
#CMAKE_BINARY_DIR - корневая директория сборки
if(CMAKE_BUILD_TYPE STREQUAL "Debug") 
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug/bin) #для исполнимых файлов
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug/lib) #для динамических библиотек
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug/lib) #для статических библиотек
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release/lib)
endif()

# Поиск Qt6, включая LinguistTools
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

# Файлы переводов
set(TS_FILES
    translates/TxtToGrdConvert_ru.ts
    # Добавьте ваши .ts файлы
)
#добавление файлов перевода
qt6_add_translations(
    TxtToGrdConvert
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES
)
include_directories(TxtToGrdConvert ${CMAKE_CURRENT_SOURCE_DIR}/includes)
#добавляем таргет библиотеки
add_library(Convert includes/Converter/Converter.h)

set(PROJECT_SOURCES
    src/main.cpp
    src/UI/mainwindow.cpp
    includes/mainwindow.h
    src/UI/mainwindow.ui
    ${QM_FILES}
)
qt_add_executable(TxtToGrdConvert
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(TxtToGrdConvert PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Convert)

if(WIN32)
set_target_properties(TxtToGrdConvert PROPERTIES
    WIN32_EXECUTABLE TRUE
)
endif()

include(GNUInstallDirs)
install(TARGETS TxtToGrdConvert
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(TxtToGrdConvert)