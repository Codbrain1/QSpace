add_library(ColorMapManager STATIC)
target_sources(ColorMapManager PRIVATE
    ColorMapManager.h
    ColorMapManager.cpp
)
target_include_directories(ColorMapManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(ColorMapManager PUBLIC
            project_config
            Qt6::Core
)