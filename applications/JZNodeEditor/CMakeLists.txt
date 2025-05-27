set(JZNodeEditor_SOURCES
    JZNodeEditor.qrc
    main.cpp      
    mainwindow.cpp    
    mainwindow.h
)

add_executable(JZNodeEditor ${JZNodeEditor_SOURCES})
target_link_libraries(JZNodeEditor JZNodeCore
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Designer
    Qt${QT_VERSION_MAJOR}::DesignerComponents
    Qt${QT_VERSION_MAJOR}::UiTools
    Qt${QT_VERSION_MAJOR}::Sql
    ${OpenCV_LIBRARIES})    
set_target_properties(JZNodeEditor PROPERTIES
    WIN32_EXECUTABLE TRUE
)