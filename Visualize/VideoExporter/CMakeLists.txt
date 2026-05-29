add_library(VideoExporter STATIC)
target_sources(VideoExporter PRIVATE
            VideoExporter.h
            VideoExporter.cpp
)
target_include_directories(VideoExporter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(VideoExporter PUBLIC
            project_config
             VTK::CommonCore
             VTK::IOVideo
             VTK::IOOggTheora
             Qt6::Concurrent
             VTK::GUISupportQt
             
)