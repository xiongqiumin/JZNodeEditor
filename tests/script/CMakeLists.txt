
set(JZNodeTest_SOURCES
    main.cpp
	test_base.cpp
	test_base.h
	test_benchmark.cpp
	test_benchmark.h
	test_debug.cpp
	test_debug.h
    test_project.cpp
	test_project.h
	test_script.cpp
	test_script.h
	test_opencv.cpp
	test_opencv.h    
    test_anglescript.cpp
    test_anglescript.h
)

add_executable(JZNodeTest ${JZNodeTest_SOURCES} ${JZNodeCore_SOURCES})
target_link_libraries(JZNodeTest Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml
	Qt${QT_VERSION_MAJOR}::Test
	Qt${QT_VERSION_MAJOR}::UiTools
    ${OpenCV_LIBRARIES})