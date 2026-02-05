add_subdirectory(Testing)
add_library(Logger STATIC)
target_sources(Logger PRIVATE
        PRIVATE Logger.h
        PUBLIC  Logger.cpp
)
target_include_directories(Logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(Logger PUBLIC
        project_config
        Qt6::Core
)   