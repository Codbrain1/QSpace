add_library(RenderLayerSettings STATIC)
target_sources(RenderLayerSettings PRIVATE
            ParticleLayer.h
            ParticleLayer.cpp
            QSpaceScalarBar.h
            QSpaceScalarBar.cpp
)
target_include_directories(RenderLayerSettings PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../.. )
target_link_libraries(RenderLayerSettings PUBLIC
            project_config
            ColorMapManager
            VTK::CommonCore
            VTK::RenderingContextOpenGL2
            VTK::ImagingHybrid
            VTK::RenderingVolumeOpenGL2
            VTK::RenderingCore             # Добавлено
            VTK::RenderingOpenGL2          # Добавлено
            Qt6::Concurrent
            VTK::FiltersPoints
)