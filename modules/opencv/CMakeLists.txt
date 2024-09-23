set(ModuleOpencv_SOURCES
    JZModuleOpencv.cpp
    JZModuleOpencv.h
)

add_library(ModuleOpencv SHARED ${ModuleOpencv_SOURCES})
target_link_libraries(ModuleOpencv PRIVATE JZNodeCore)
set_target_properties(ModuleOpencv PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
)