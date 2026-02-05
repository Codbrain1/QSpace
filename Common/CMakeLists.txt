

add_subdirectory(Enums)
add_subdirectory(Interfaces)
add_subdirectory(Logger)
# add_subdirectory(PhysicalConstants)
add_subdirectory(Structures)
# add_subdirectory(Version)

add_library(Common INTERFACE)
add_library(QSpace::Common ALIAS Common)
target_include_directories(Common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Common INTERFACE
    Enums
    Interfaces
    Logger
    # PhysicalConstants
    Structures
    # Version
    project_config
)

