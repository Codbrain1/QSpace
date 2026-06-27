add_subdirectory(ColorMapManager)
add_subdirectory(RenderLayerSettings)
add_subdirectory(VideoExporter)
add_library(Visualize STATIC)
add_library(QSpace::Visualize ALIAS Visualize)

target_sources(Visualize PRIVATE 
            VtkView.h
            VtkView.cpp
            GalacticInteractorStyle.h
            GalacticInteractorStyle.cpp
            LayerFactory.cpp
)

target_include_directories(Visualize PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Visualize PUBLIC
            project_config
            RenderLayerSettings
            VideoExporter
            ColorMapManager
            VTK::GUISupportQt
            QSpace::Common
            Qt6::Core
            Qt6::Widgets
            Qt6::OpenGLWidgets
            VTK::CommonCore
            VTK::RenderingAnnotation
            VTK::InteractionStyle
            VTK::CommonColor
            VTK::RenderingCore
)