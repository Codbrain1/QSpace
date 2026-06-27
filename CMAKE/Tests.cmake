find_package(Qt6 REQUIRED COMPONENTS Test Concurrent)
function(qspace_add_test TestName)
    set(test_libs ${ARGN})
    add_executable(${TestName} Cxx/${TestName}.cpp)

    target_link_libraries(${TestName} PRIVATE 
    Qt6::Test
    Qt6::Concurrent
    ${test_libs}    
)

    add_test(NAME ${TestName} COMMAND ${TestName})
    set(QT_BIN_DIR "C:/Qt/6.11.0/mingw_64/bin")
    set(VTK_BIN_DIR "D:/NIR/NIR_6_semestr/vtk/vtk-install-ffmpeg-wmf-qt6_11-dll-OpenMP/bin")
    set_tests_properties(${TestName} PROPERTIES ENVIRONMENT 
        "PATH=${QT_BIN_DIR}\;${VTK_BIN_DIR}\;$ENV{PATH}"
    )
endfunction(qspace_add_test TestName)

# Определяем функцию для быстрой регистрации теста
function(add_sandbox_test NAME SOURCE_FILE)
    add_executable(${NAME} ${SOURCE_FILE})
    
    set_target_properties(${NAME} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
    
    target_link_libraries(${NAME} PRIVATE
        QSpace::Common
        Qt6::Core
        Qt6::Widgets
        VTK::CommonCore
        VTK::RenderingOpenGL2
        VTK::InteractionStyle
        ${ARGN} # Сюда попадут дополнительные библиотеки, если мы их передадим
    )
endfunction()
