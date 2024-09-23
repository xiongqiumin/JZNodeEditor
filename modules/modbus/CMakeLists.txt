include_directories(jzmodbus)

set(ModuleModbus_SOURCES    
    jzmodbus/JZModbusClient.cpp
    jzmodbus/JZModbusClient.h
    jzmodbus/JZModbusContext.cpp
    jzmodbus/JZModbusContext.h
    jzmodbus/JZModbusDefine.cpp
    jzmodbus/JZModbusDefine.h
    jzmodbus/JZModbusMaster.cpp
    jzmodbus/JZModbusMaster.h
    jzmodbus/JZModbusParam.cpp
    jzmodbus/JZModbusParam.h
    jzmodbus/JZModbusRtu.cpp
    jzmodbus/JZModbusRtu.h
    jzmodbus/JZModbusServer.cpp
    jzmodbus/JZModbusServer.h
    jzmodbus/JZModbusSlaver.cpp
    jzmodbus/JZModbusSlaver.h
    jzmodbus/JZModbusTcp.cpp
    jzmodbus/JZModbusTcp.h

    JZModuleModbus.cpp
    JZModuleModbus.h
    JZModbusConfigDialog.cpp
    JZModbusConfigDialog.h
    JZModbusConfigDialog.ui
    JZModbusSimulator.cpp
    JZModbusSimulator.h
    JZModbusTemplateDialog.cpp
    JZModbusTemplateDialog.h
)


add_library(ModuleModbus SHARED ${ModuleModbus_SOURCES})
target_link_libraries(ModuleModbus PRIVATE JZNodeCore)
set_target_properties(ModuleModbus PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/modules"
)

add_dependencies(JZNodeEditor ModuleModbus)