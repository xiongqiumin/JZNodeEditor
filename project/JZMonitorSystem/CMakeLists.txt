set(JZMonitorSystem_SOURCES
    JZMonitorSystem.qrc
    main.cpp      
    mainwindow.cpp    
    mainwindow.h
    flowTreeWidget.cpp
    flowTreeWidget.h
    JZTitleWidget.cpp
    JZTitleWidget.h
    JZPanelWidget.cpp
    JZPanelWidget.h
)

add_executable(JZMonitorSystem ${JZMonitorSystem_SOURCES})
target_link_libraries(JZMonitorSystem JZNodeCore
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml    
    Qt${QT_VERSION_MAJOR}::UiTools
    Qt${QT_VERSION_MAJOR}::Sql
    ${OpenCV_LIBRARIES})    
set_target_properties(JZMonitorSystem PROPERTIES
    WIN32_EXECUTABLE TRUE
)