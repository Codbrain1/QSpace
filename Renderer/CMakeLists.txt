add_subdirectory(ParticleLayer)
add_library(Renderer STATIC)
add_library(QSpace::Renderer ALIAS Renderer)
target_sources(Renderer PRIVATE 
            Renderer.h
            Renderer.cpp
            LayerFactory.cpp
)

target_include_directories(Renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Renderer PUBLIC
            project_config
            ParticleLayer
            QSpace::Common
            Qt6::Core
            Qt6::Widgets
            VTK::CommonCore
            VTK::RenderingAnnotation
)