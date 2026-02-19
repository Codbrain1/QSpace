add_library(AppCore STATIC)
target_sources(AppCore PRIVATE 
            AppCore.h
            AppCore.cpp
)
target_include_directories(AppCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(AppCore PUBLIC 
                project_config
                QSpace::Common
                QSpace::IO
                QSpace::Renderer
                ObjectRegistry
                DataManager
                PipelineManager
                TaskManager
                Qt6::Core
)