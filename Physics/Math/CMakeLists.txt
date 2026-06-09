add_library(Math STATIC)
target_sources(Math PRIVATE
            MetaDataCalculating.h
            MetaDataCalculating.cpp
)
target_include_directories(Math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(Math PUBLIC
            project_config
            QSpace::Common
            VTK::CommonCore
            VTK::CommonDataModel
) 