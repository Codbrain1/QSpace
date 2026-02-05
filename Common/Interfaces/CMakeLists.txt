add_library(Interfaces INTERFACE)
target_sources(Interfaces INTERFACE
            IReader.h
            IWriter.h
            IOFactory.h
)
target_include_directories(Interfaces INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Interfaces INTERFACE
            project_config
)