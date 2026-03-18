add_subdirectory(ColorMapManager)
add_subdirectory(RenderLayerSettings)
add_subdirectory(VideoExporter)
add_library(Visualize STATIC)
add_library(QSpace::Visualize ALIAS Visualize)

target_sources(Visualize PRIVATE 
            Renderer.h
            Renderer.cpp
            LayerFactory.cpp
)

target_include_directories(Visualize PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Visualize PUBLIC
            project_config
            RenderLayerSettings
            VideoExporter
            ColorMapManager
            QSpace::Common
            Qt6::Core
            Qt6::Widgets
            VTK::CommonCore
            VTK::RenderingAnnotation
)