set(JZVisionPlatform_SOURCES
    JZVisionPlatform.qrc
    main.cpp      
    mainwindow.cpp    
    mainwindow.h
    JZTitleWidget.cpp
    JZTitleWidget.h
    JZPanelWidget.cpp
    JZPanelWidget.h
	JZCameraListWidget.cpp
	JZCameraListWidget.h
	JZCameraViewWidget.cpp
	JZCameraViewWidget.h
	JZModelVisionApp.cpp
	JZModelVisionApp.h
)

add_executable(JZVisionPlatform ${JZVisionPlatform_SOURCES})
target_link_libraries(JZVisionPlatform JZNodeCore JZNodeEditorCore
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml    
    Qt${QT_VERSION_MAJOR}::UiTools
    Qt${QT_VERSION_MAJOR}::Sql
    ${OpenCV_LIBRARIES})    
set_target_properties(JZVisionPlatform PROPERTIES
    WIN32_EXECUTABLE TRUE
)