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
    set(VTK_BIN_DIR "D:/NIR/NIR_6_semestr/vtk/vtk-install-ffmpeg-wmf-dll/bin")
    set_tests_properties(${TestName} PROPERTIES ENVIRONMENT 
        "PATH=${QT_BIN_DIR}\;${VTK_BIN_DIR}\;$ENV{PATH}"
    )
endfunction(qspace_add_test TestName)
