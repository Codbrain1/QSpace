add_subdirectory(OpenGL3DWidget)

add_library(View3D STATIC)
target_sources(View3D PRIVATE
        View3DSettings.h
        AbstractView3D.h
)
target_include_directories(View3D PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(View3D PUBLIC
            project_config
            QSpace::Common
            Qt6::Core
            Qt6::Widgets
            Qt6::Gui
)