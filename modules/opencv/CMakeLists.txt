find_package(OpenCV 4 QUIET)

if(${OpenCV_FOUND})

include_directories(${OpenCV_INCLUDE_DIRS})

set(ModuleOpencv_SOURCES
    CvMatAndQImage.cpp
    CvMatAndQImage.h
    JZModuleOpencv.cpp
    JZModuleOpencv.h
)

add_library(ModuleOpencv SHARED ${ModuleOpencv_SOURCES})
target_link_libraries(ModuleOpencv PRIVATE JZNodeCore ${OpenCV_LIBRARIES})
set_target_properties(ModuleOpencv PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
)

add_dependencies(JZNodeEditor ModuleOpencv)
else()

message("not find opencv, skip build ModuleOpencv.")

endif()