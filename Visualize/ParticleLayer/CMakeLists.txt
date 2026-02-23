add_library(ParticleLayer STATIC)
target_sources(ParticleLayer PRIVATE
            ParticleLayer.h
            ParticleLayer.cpp
)
target_include_directories(ParticleLayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(ParticleLayer PUBLIC
            project_config
             VTK::CommonCore
             Qt6::Concurrent
)