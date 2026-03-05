add_library(Session STATIC)
add_library(QSpace::Session ALIAS Session)
target_sources(Session PRIVATE 
    ProjectSerializer.h
    ProjectSerializer.cpp
    SessionStorage.h
    SessionStorage.cpp
)

target_include_directories(Session PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Session PUBLIC
            project_config
            QSpace::Common
            Qt6::Core
            Qt6::Widgets
)