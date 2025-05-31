set(JZVisionPlatform_SOURCES
    JZVisionPlatform.qrc
    main.cpp      
    mainwindow.cpp    
    mainwindow.h
    JZTitleWidget.cpp
    JZTitleWidget.h
    JZPanelWidget.cpp
    JZPanelWidget.h
    JZEventWidget.cpp
    JZEventWidget.h
	JZCameraListWidget.cpp
	JZCameraListWidget.h
	JZCameraViewWidget.cpp
	JZCameraViewWidget.h
	JZModuleVisionApp.cpp
	JZModuleVisionApp.h
    JZFlowTree.cpp
    JZFlowTree.h
	JZCameraThread.cpp
	JZCameraThread.h
    
    editor/JZVisionPanel.cpp
    editor/JZVisionPanel.h
	editor/JZVisionEditor.cpp
	editor/JZVisionEditor.h
	editor/JZVisionImage.cpp
	editor/JZVisionImage.h	
	editor/JZVisionLineItem.cpp
	editor/JZVisionLineItem.h
	editor/JZVisionNodeItem.cpp
	editor/JZVisionNodeItem.h
	editor/JZVisionOutput.cpp
	editor/JZVisionOutput.h	
	editor/JZVisionSettingDialog.cpp
	editor/JZVisionSettingDialog.h
    editor/JZVisionEditorManager.cpp
    editor/JZVisionEditorManager.h
	editor/JZVisionView.cpp
	editor/JZVisionView.h
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