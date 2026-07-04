add_library(OpenGL3DWidget STATIC)
target_sources(OpenGL3DWidget PRIVATE
        OpenGL3DWidget.h
        OpenGL3DWidget.cpp
        AxisRenderer.h
        AxisRenderer.cpp
        GridRenderer.h
        GridRenderer.cpp
        GLViewport.h
        GLViewport.cpp
)
target_include_directories(OpenGL3DWidget PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(OpenGL3DWidget PUBLIC
            project_config
            QSpace::Common
            Qt6::Core
            Qt6::OpenGL
            Qt6::OpenGLWidgets
)