# 1. Настройка переводов
#------------------------------------------
set(TS_FILES
    ${PROJECT_SOURCE_DIR}/app/translates/QSpace_ru.ts
)
#добавление файлов перевода
qt6_add_translations(
    QSpace
    TS_FILES ${TS_FILES}
RESOURCE_PREFIX "/i18n"
)

#создание исполнимого qt файла
qt_add_executable(QSpace
    MANUAL_FINALIZATION
    main.cpp
    ${QM_FILES}
)

target_link_libraries(QSpace PRIVATE
            Common
            IO
            Core
            Charting
            Common
            Core
            Filtering
            IO
            Physics
            Remote
            Renderer
            Session
            UI
            project_config
            Qt6::Widgets
)

if(WIN32) # на виндовс только gui
set_target_properties(QSpace PROPERTIES
    WIN32_EXECUTABLE TRUE
)

add_custom_command(TARGET QSpace POST_BUILD
COMMAND "C:/Qt/6.9.2/mingw_64/bin/windeployqt.exe"
    --compiler-runtime
    $<TARGET_FILE:QSpace>
        COMMENT "Running windeployqt..."
        )

endif()

include(GNUInstallDirs)
install(TARGETS QSpace
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/" 
        DESTINATION bin 
        FILES_MATCHING 
        PATTERN "*.dll"        # Забираем библиотеки
        PATTERN "platforms/*"  # Забираем критический плагин qwindows.dll
        PATTERN "styles/*"
        PATTERN "imageformats/*"
)

qt_finalize_executable(QSpace)