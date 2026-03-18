set(CMAKE_AUTOUIC ON)
add_library(UI STATIC)
add_library(QSpace::UI ALIAS UI)
target_sources(UI PRIVATE
    Home/MainWindow.h
    Home/MainWindow.cpp
    Home/newmainwindow.ui
    Home/DataTreeController.h
    Home/DataTreeController.cpp
    Home/PropertyInspector.h
    Home/PropertyInspector.cpp
    Home/PropertyInspector.ui
    Home/ColorMapComboBox.h
    Home/ColorMapComboBox.cpp
    Home/ColorMapEditorDialog.h
    Home/ColorMapEditorDialog.cpp
    Home/ColorMapEditorDialog.ui
)
target_include_directories(UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(UI PUBLIC
    QSpace::Core
    QSpace::Visualize
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    VTK::GUISupportQt
)