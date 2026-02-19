add_library(DataManager STATIC)
target_sources(DataManager PRIVATE 
            DataManager.h
            DataManager.cpp
)   
target_include_directories(DataManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(DataManager PUBLIC 
                project_config
                QSpace::IO
                QSpace::Common
                Qt6::Core
                Qt6::Concurrent
)