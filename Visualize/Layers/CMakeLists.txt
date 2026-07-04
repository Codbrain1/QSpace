add_library(Layers STATIC)
target_sources(Layers PRIVATE
           Layer.h
)
target_include_directories(Layers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(Layers PUBLIC
            project_config

            Qt6::Core
            Qt6::Widgets
            Qt6::Gui
)