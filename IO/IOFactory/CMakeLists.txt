add_library(IOFactory STATIC)
target_sources(IOFactory PRIVATE 
            IOFactory.cpp
)           
target_include_directories(IOFactory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(IOFactory PUBLIC 
            project_config
            QSpace::Common
            IO_BIN
)