add_subdirectory(DataTreeModel)

add_library(Presentation INTERFACE)
add_library(QSpace::Presentation ALIAS Presentation)
target_include_directories(Presentation INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Presentation INTERFACE
            project_config
            QSpace::Core
            DataTreeModel
            # SceneSettingsModel
            # SessionModel
            # TaskProgressModel
            Qt6::Core
)