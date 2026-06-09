add_subdirectory(Math)
add_library(Physics INTERFACE)
add_library(QSpace::Physics ALIAS Physics)
target_include_directories(Physics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Physics INTERFACE
    project_config
    Math
)