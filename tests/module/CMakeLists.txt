
include_directories(../script)

set(JZNodeModuleTest_SOURCES
    main.cpp
	../script/test_base.cpp
	../script/test_base.h
    test_camera.cpp
    test_camera.h
	test_modbus.cpp
	test_modbus.h
)

add_executable(JZNodeModuleTest ${JZNodeModuleTest_SOURCES})
target_link_libraries(JZNodeModuleTest JZNodeCore
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml
	Qt${QT_VERSION_MAJOR}::Test
	Qt${QT_VERSION_MAJOR}::UiTools
    ${OpenCV_LIBRARIES})