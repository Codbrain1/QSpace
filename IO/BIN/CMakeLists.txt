add_subdirectory(Testing)
add_library(IO_BIN STATIC)
target_sources(IO_BIN PRIVATE
    BINReader.h
    BINReader.cpp
    # BINWriter.h
    # BINWriter.cpp
)
target_include_directories(IO_BIN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(IO_BIN PUBLIC 
                    project_config
                    QSpace::Common
                    Qt6::Core
)