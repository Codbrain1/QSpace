add_library(ViewManager STATIC)
target_sources(ViewManager PRIVATE
        ViewManager.h
        ViewManager.cpp
)
target_include_directories(ViewManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(ViewManager PUBLIC 
            project_config
            Qt6::Core
            QSpace::Renderer
)   