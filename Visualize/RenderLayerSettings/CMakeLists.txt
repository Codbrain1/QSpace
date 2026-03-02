add_library(RenderLayerSettings STATIC)
target_sources(RenderLayerSettings PRIVATE
            ParticleLayer.h
            ParticleLayer.cpp
)
target_include_directories(RenderLayerSettings PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(RenderLayerSettings PUBLIC
            project_config
             VTK::CommonCore
             Qt6::Concurrent
)