add_subdirectory(Testing)
add_library(PipelineManager STATIC)
target_sources(PipelineManager PRIVATE 
PipelineManager.h
PipelineManager.cpp
)
target_include_directories(PipelineManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(PipelineManager PUBLIC
            project_config
            ObjectRegistry
            LayerManager
            Qt6::Core
            Qt6::Widgets
)