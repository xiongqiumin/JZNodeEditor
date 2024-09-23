

set(JZTest_SOURCES    
    main.cpp
    test_anglescript.cpp
    test_anglescript.h
    test_base.cpp
    test_base.h
    test_benchmark.cpp
    test_benchmark.h
    test_server.cpp
    test_server.h
    test_script.cpp
    test_script.h
)

add_executable(test ${JZTest_SOURCES})
target_link_libraries(test JZNodeCore Qt${QT_VERSION_MAJOR}::Test)
set_target_properties(test PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)