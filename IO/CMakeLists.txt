add_subdirectory(BIN)
add_subdirectory(GRD)
add_subdirectory(IOFactory)
# add_subdirectory(HDF5)
# add_subdirectory(VTK)
add_library(IO INTERFACE)
add_library(QSpace::IO ALIAS IO)
target_include_directories(IO INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(IO INTERFACE
                    IO_BIN
                    IO_GRD
                    IOFactory
                    # IO_HDF5
                    # IO_VTK
)