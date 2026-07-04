# TODO
# add_subdirectory(View2D) 

add_subdirectory(View3D)

add_library(Views STATIC)
target_sources(Views PRIVATE
            AbstractView.h
            ViewFactory.h
            ViewFactory.cpp
)
target_include_directories(Views PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(Views PUBLIC
            project_config
            QSpace::Common
            View3D
)