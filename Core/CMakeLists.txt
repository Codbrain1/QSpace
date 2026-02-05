add_subdirectory(LogManager)
add_subdirectory(DataManager)
add_subdirectory(TaskManager)
add_library(Core INTERFACE)
target_include_directories(Core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Core INTERFACE
            project_config
            DataManager
            LogManager
            TaskManager
            Qt6::Core
)