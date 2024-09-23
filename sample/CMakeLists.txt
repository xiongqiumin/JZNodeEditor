set(JZSample_SOURCES    
    main.cpp
    SampleProject.cpp
    SampleProject.h        
    Russian/Russian.cpp
    Russian/Russian.h  
    SmartHome/SmartHome.cpp
    SmartHome/SmartHome.h  
)

add_executable(gen_sample ${JZSample_SOURCES})
target_link_libraries(gen_sample JZNodeCore Qt${QT_VERSION_MAJOR}::Xml)
set_target_properties(gen_sample PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

set(ModuleImageSample_SOURCES
    ImageModule/ImageModule.cpp
    ImageModule/ImageModule.h
)

add_library(ModuleImageSample SHARED ${ModuleImageSample_SOURCES})
target_link_libraries(ModuleImageSample PRIVATE JZNodeCore)
set_target_properties(ModuleImageSample PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
)