cmake_minimum_required(VERSION 3.5)

project(JZNodeEditor)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS widgets xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS core gui network widgets uitools serialport test printsupport xml sql)
find_package(OpenCV 4 REQUIRED)

if (MSVC)
    add_compile_options(/execution-charset:utf-8)
    add_compile_options(/bigobj)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)

include_directories(./
    ./widgets
    ./script
    ./editor
    ./editor/ui
    ./editor/script
    ./editor/script/items   
    ./3rd/JZCommon
    ${OpenCV_INCLUDE_DIRS})

set(JZNodeCore_SOURCES
    LogManager.cpp
    LogManager.h
	
	script/JZParamHelper.cpp
	script/JZParamHelper.h
    script/JZParamItem.cpp
    script/JZParamItem.h
    script/JZProject.cpp
    script/JZProject.h
    script/JZProjectItem.cpp
    script/JZProjectItem.h    
    script/JZProjectTemplate.cpp
    script/JZProjectTemplate.h
    script/JZScriptFile.cpp
    script/JZScriptFile.h
    script/JZScriptItem.cpp
    script/JZScriptItem.h
    script/JZClassItem.cpp
    script/JZClassItem.h
    script/JZProjectFile.cpp
    script/JZProjectFile.h
    script/JZUiBaseItem.cpp
    script/JZUiBaseItem.h  
    script/JZUiItem.cpp
    script/JZUiItem.h    
    script/JZContainer.cpp
    script/JZContainer.h
    script/JZExpressionSolver.cpp
    script/JZExpressionSolver.h
    script/JZModule.cpp
    script/JZModule.h
    script/JZNode.cpp
    script/JZNode.h
    script/JZNodeFlow.cpp
    script/JZNodeFlow.h
    script/JZNodeBind.cpp
    script/JZNodeBind.h
    script/JZNodeBuilder.cpp
    script/JZNodeBuilder.h
    script/JZNodeCompiler.cpp
    script/JZNodeCompiler.h    
    script/JZNodeDebugClient.cpp
    script/JZNodeDebugClient.h
    script/JZNodeDebugPacket.cpp
    script/JZNodeDebugPacket.h
    script/JZNodeDebugServer.cpp
    script/JZNodeDebugServer.h
    script/JZNodeEngine.cpp
    script/JZNodeEngine.h
    script/JZNodeEnum.cpp
    script/JZNodeEnum.h
    script/JZNodeEvent.cpp
    script/JZNodeEvent.h
    script/JZNodeExpression.cpp
    script/JZNodeExpression.h
    script/JZNodeFactory.cpp
    script/JZNodeFactory.h
    script/JZNodeFunction.cpp
    script/JZNodeFunction.h
    script/JZNodeFunctionDefine.cpp
    script/JZNodeFunctionDefine.h
    script/JZNodeFunctionManager.cpp
    script/JZNodeFunctionManager.h
    script/JZScriptEnvironment.cpp
    script/JZScriptEnvironment.h
    script/JZNodeInit.cpp
    script/JZNodeInit.h
    script/JZNodeIR.cpp
    script/JZNodeIR.h
    script/JZNodeLangServer.cpp
    script/JZNodeLangServer.h
    script/JZNodeMeta.cpp
    script/JZNodeMeta.h
    script/JZNodeObject.cpp
    script/JZNodeObject.h
    script/JZNodeObjectParser.cpp
    script/JZNodeObjectParser.h
    script/JZNodeOperator.cpp
    script/JZNodeOperator.h
    script/JZNodePin.cpp
    script/JZNodePin.h
    script/JZNodeProgram.cpp
    script/JZNodeProgram.h
    script/JZNodeProgramDumper.cpp
    script/JZNodeProgramDumper.h
    script/JZNodeQtWrapper.cpp
    script/JZNodeQtWrapper.h
    script/JZNodeType.cpp
    script/JZNodeType.h        
    script/JZNodeUtils.cpp
    script/JZNodeUtils.h
    script/JZNodeValue.cpp
    script/JZNodeValue.h
    script/JZNodeVM.cpp
    script/JZNodeVM.h
    script/JZRegExpHelp.cpp
    script/JZRegExpHelp.h
    script/JZFormat.cpp
    script/JZFormat.h
    script/JZScriptBuildInFunction.cpp
    script/JZScriptBuildInFunction.h
    script/JZScriptUnitTest.cpp
    script/JZScriptUnitTest.h
    script/JZScriptItemVisitor.cpp
    script/JZScriptItemVisitor.h
    script/JZScriptConvert.cpp
    script/JZScriptConvert.h      
    script/mvvm/JZNodeVariableBind.cpp
    script/mvvm/JZNodeVariableBind.h
    script/angelscript/as_parser.cpp
    script/angelscript/as_parser.h
    script/angelscript/as_scriptcode.cpp
    script/angelscript/as_scriptcode.h
    script/angelscript/as_scriptnode.cpp
    script/angelscript/as_scriptnode.h
    script/angelscript/as_tokendef.h
    script/angelscript/as_tokenizer.cpp
    script/angelscript/as_tokenizer.h    

    #runtime
    runtime/JZNodeUiLoader.cpp
    runtime/JZNodeUiLoader.h
    runtime/JZWidgetBind.cpp
    runtime/JZWidgetBind.h
	
	#modules
	modules/JZModuleConfigWidget.cpp
	modules/JZModuleConfigWidget.h
    
    #communication        
    modules/communication/JZModuleComm.cpp
    modules/communication/JZModuleComm.h
    modules/communication/JZComm.cpp
    modules/communication/JZComm.h
    modules/communication/JZCommNode.cpp
    modules/communication/JZCommNode.h
    modules/communication/JZCommManager.cpp
    modules/communication/JZCommManager.h
    modules/communication/JZCommPack.cpp
    modules/communication/JZCommPack.h
	modules/communication/JZCommWidget.cpp
    modules/communication/JZCommWidget.h
    modules/communication/modbus/JZCommModbusClient.cpp
    modules/communication/modbus/JZCommModbusClient.h
    modules/communication/modbus/JZCommModbusServer.cpp
    modules/communication/modbus/JZCommModbusServer.h
    modules/communication/net/JZTcpClient.cpp
    modules/communication/net/JZTcpClient.h
    modules/communication/net/JZTcpServer.cpp
    modules/communication/net/JZTcpServer.h
    modules/communication/net/JZUdpSocket.cpp
    modules/communication/net/JZUdpSocket.h
    modules/communication/serialPort/JZSerialPort.cpp
    modules/communication/serialPort/JZSerialPort.h
    
    #opencv
    modules/opencv/JZModuleOpencv.h
    modules/opencv/JZModuleOpencv.cpp    
    modules/opencv/CvMatAndQImage.h
    modules/opencv/CvMatAndQImage.cpp
    modules/opencv/CvToQt.h
    modules/opencv/CvToQt.cpp
	modules/opencv/JZOpencvNode.h
    modules/opencv/JZOpencvNode.cpp
    modules/opencv/JZOpencv.h
    modules/opencv/JZOpencv.cpp
	
	#model
    modules/model/JZYolo.h
    modules/model/JZYolo.cpp
    modules/model/JZYoloView.h
    modules/model/JZYoloView.cpp
    modules/model/JZModel.h
    modules/model/JZModel.cpp
    modules/model/JZModelNode.cpp
	modules/model/JZModelNode.h
	modules/model/JZModelManager.h
	modules/model/JZModelManager.cpp
	modules/model/JZModelWidget.cpp
    modules/model/JZModelWidget.h
	modules/model/JZModuleModel.h
    modules/model/JZModuleModel.cpp    

    #camera
    modules/camera/JZModuleCamera.h
    modules/camera/JZModuleCamera.cpp
    modules/camera/JZCamera.h
    modules/camera/JZCamera.cpp
	modules/camera/JZCameraUVC.h   
	modules/camera/JZCameraUVC.cpp    
    modules/camera/JZCameraRtsp.cpp
    modules/camera/JZCameraRtsp.h
    modules/camera/JZCameraHik.h
    modules/camera/JZCameraHik.cpp
    modules/camera/JZCameraFile.h
    modules/camera/JZCameraFile.cpp   
    modules/camera/JZCameraNode.h
    modules/camera/JZCameraNode.cpp
    modules/camera/JZCameraManager.h
    modules/camera/JZCameraManager.cpp
	modules/camera/JZCameraWidget.cpp
    modules/camera/JZCameraWidget.h
    modules/camera/JZCameraUnitTest.h
    modules/camera/JZCameraUnitTest.cpp
    
    #vision
    modules/vision/JZModuleVision.cpp
    modules/vision/JZModuleVision.h
    modules/vision/JZVision.cpp
    modules/vision/JZVision.h
    modules/vision/JZVisionNode.cpp
    modules/vision/JZVisionNode.h  
    modules/vision/JZShapeMatch.cpp
    modules/vision/JZShapeMatch.h
    modules/vision/JZTemplateMatch.cpp
    modules/vision/JZTemplateMatch.h    
    
    #motion
    modules/motion/JZModuleMotion.cpp
    modules/motion/JZModuleMotion.h
    modules/motion/JZMotion.cpp
    modules/motion/JZMotion.h
    modules/motion/JZMotionNode.cpp
    modules/motion/JZMotionNode.h
    
    #widgets
    widgets/JZBaseDialog.cpp
    widgets/JZBaseDialog.h
    widgets/JZPropertyDialog.cpp
    widgets/JZPropertyDialog.h
    
    #3rd
    3rd/JZCommon/jzNet/JZNetClient.cpp
    3rd/JZCommon/jzNet/JZNetClient.h
    3rd/JZCommon/jzNet/JZNetDataManager.cpp
    3rd/JZCommon/jzNet/JZNetDataManager.h
    3rd/JZCommon/jzNet/JZNetPack.cpp
    3rd/JZCommon/jzNet/JZNetPack.h
    3rd/JZCommon/jzNet/JZNetServer.cpp
    3rd/JZCommon/jzNet/JZNetServer.h
    3rd/JZCommon/jzProfiler/JZTx.cpp
    3rd/JZCommon/jzProfiler/JZTx.h
    3rd/JZCommon/jzModbus/JZModbusClient.cpp
    3rd/JZCommon/jzModbus/JZModbusClient.h
    3rd/JZCommon/jzModbus/JZModbusContext.cpp
    3rd/JZCommon/jzModbus/JZModbusContext.h
    3rd/JZCommon/jzModbus/JZModbusDefine.cpp
    3rd/JZCommon/jzModbus/JZModbusDefine.h
    3rd/JZCommon/jzModbus/JZModbusMaster.cpp
    3rd/JZCommon/jzModbus/JZModbusMaster.h
    3rd/JZCommon/jzModbus/JZModbusParam.cpp
    3rd/JZCommon/jzModbus/JZModbusParam.h
    3rd/JZCommon/jzModbus/JZModbusRtu.cpp
    3rd/JZCommon/jzModbus/JZModbusRtu.h
    3rd/JZCommon/jzModbus/JZModbusServer.cpp
    3rd/JZCommon/jzModbus/JZModbusServer.h
    3rd/JZCommon/jzModbus/JZModbusSlaver.cpp
    3rd/JZCommon/jzModbus/JZModbusSlaver.h
    3rd/JZCommon/jzModbus/JZModbusTcp.cpp
    3rd/JZCommon/jzModbus/JZModbusTcp.h
    3rd/JZCommon/jzWidgets/JZImageView.cpp
    3rd/JZCommon/jzWidgets/JZImageView.h    
    3rd/JZCommon/jzWidgets/JZImageLabel.cpp 
    3rd/JZCommon/jzWidgets/JZImageLabel.h     
    3rd/JZCommon/jzWidgets/JZLogWidget.cpp
    3rd/JZCommon/jzWidgets/JZLogWidget.h
    3rd/JZCommon/jzWidgets/JZSelfLayout.cpp
    3rd/JZCommon/jzWidgets/JZSelfLayout.h    
    3rd/JZCommon/jzWidgets/JZPropertyBrowser.cpp
    3rd/JZCommon/jzWidgets/JZPropertyBrowser.h
    3rd/JZCommon/jzWidgets/JZPropertyEditor.cpp
    3rd/JZCommon/jzWidgets/JZPropertyEditor.h
    3rd/JZCommon/jzWidgets/JZPropertyWidget.cpp
    3rd/JZCommon/jzWidgets/JZPropertyWidget.h  
    
    3rd/JZCommon/jzJson/JZJsonStream.cpp
    3rd/JZCommon/jzJson/JZJsonStream.h
    3rd/JZCommon/jzJson/JZJsonHelper.cpp
    3rd/JZCommon/jzJson/JZJsonHelper.h
    3rd/JZCommon/jzCommon/JZLogManager.cpp
    3rd/JZCommon/jzCommon/JZLogManager.h
)

set(JZNodeEditorCore_SOURCES
    JZNodeEditor.qrc     
    mainTask.cpp    
    mainTask.h    
    LogWidget.cpp
    LogWidget.h    
    UiCommon.cpp
    UiCommon.h   
    database.cpp
    database.h    
    editor/JZEditor.cpp
    editor/JZEditor.h
    editor/JZEditorGlobal.cpp
    editor/JZEditorGlobal.h
    editor/JZNewFileDialog.cpp
    editor/JZNewFileDialog.h
    editor/JZNewFileDialog.ui
    editor/JZNewProjectDialog.cpp
    editor/JZNewProjectDialog.h
    editor/JZNewProjectDialog.ui
    editor/JZProjectSettingDialog.cpp
    editor/JZProjectSettingDialog.h
    editor/JZProjectTree.cpp
    editor/JZProjectTree.h
    editor/JZEditorUtils.cpp
    editor/JZEditorUtils.h
    editor/JZIconManager.cpp
    editor/JZIconManager.h
    editor/script/JZNodeAutoRunThread.cpp
    editor/script/JZNodeAutoRunThread.h
    editor/script/JZNodeAutoRunWidget.cpp
    editor/script/JZNodeAutoRunWidget.h    
    editor/script/JZNodeBreakPointWidget.cpp
    editor/script/JZNodeBreakPointWidget.h
    editor/script/JZNodeBuildThread.cpp
    editor/script/JZNodeBuildThread.h
    editor/script/JZNodeEditor.cpp
    editor/script/JZNodeEditor.h
    editor/script/JZNodeEditorManager.cpp
    editor/script/JZNodeEditorManager.h
    editor/script/JZNodeExprEditDialog.cpp
    editor/script/JZNodeExprEditDialog.h
    editor/script/JZNodeFlagEditDialog.cpp
    editor/script/JZNodeFlagEditDialog.h
    editor/script/JZNodeFuctionEditDialog.cpp
    editor/script/JZNodeFuctionEditDialog.h
    editor/script/JZNodeFuctionEditDialog.ui    
    editor/script/JZNodeGroupEditDialog.cpp
    editor/script/JZNodeGroupEditDialog.h
    editor/script/JZNodeGroupEditDialog.ui    
    editor/script/JZNodeLayout.cpp
    editor/script/JZNodeLayout.h
    editor/script/JZNodeLocalParamEditDialog.cpp
    editor/script/JZNodeLocalParamEditDialog.h
    editor/script/JZNodeMemberSelectDialog.cpp
    editor/script/JZNodeMemberSelectDialog.h
    editor/script/JZNodeMemberSelectDialog.ui
    editor/script/JZNodePanel.cpp
    editor/script/JZNodePanel.h
    editor/script/JZNodeParamBindEditDialog.cpp
    editor/script/JZNodeParamBindEditDialog.h
    editor/script/JZNodeParamBindEditDialog.ui
    editor/script/JZNodeParamEditor.cpp
    editor/script/JZNodeParamEditor.h
    editor/script/JZNodeParamEditor.ui    
    editor/script/JZNodeParamEditWidget.cpp
    editor/script/JZNodeParamEditWidget.h
    editor/script/JZNodeParamDisplayWidget.cpp
    editor/script/JZNodeParamDisplayWidget.h        
    editor/script/JZNodePropertyEditor.cpp
    editor/script/JZNodePropertyEditor.h
    editor/script/JZNodeScene.cpp
    editor/script/JZNodeScene.h
    editor/script/JZNodeSlotEditDialog.cpp
    editor/script/JZNodeSlotEditDialog.h
    editor/script/JZNodeStack.cpp
    editor/script/JZNodeStack.h
    editor/script/JZNodeTypeDialog.ui
    editor/script/JZNodeTypeHelper.cpp
    editor/script/JZNodeTypeHelper.h
    editor/script/JZNodeView.cpp
    editor/script/JZNodeView.h
    editor/script/JZNodeViewCommand.cpp
    editor/script/JZNodeViewCommand.h
    editor/script/JZNodeViewMap.cpp
    editor/script/JZNodeViewMap.h
    editor/script/JZNodeFlowPanel.cpp
    editor/script/JZNodeFlowPanel.h
    editor/script/JZNodeVirtualFuctionEditDialog.cpp
    editor/script/JZNodeVirtualFuctionEditDialog.h
    editor/script/JZNodeWatch.cpp
    editor/script/JZNodeWatch.h
    editor/script/JZProcess.cpp
    editor/script/JZProcess.h   
    editor/script/JZNodeClassEditDialog.ui
    editor/script/JZNodeClassEditDialog.cpp
    editor/script/JZNodeClassEditDialog.h       
    editor/script/items/JZNodeBaseItem.cpp
    editor/script/items/JZNodeBaseItem.h
    editor/script/items/JZNodeGroupItem.cpp
    editor/script/items/JZNodeGroupItem.h
    editor/script/items/JZNodeLineItem.cpp
    editor/script/items/JZNodeLineItem.h
    editor/script/items/JZNodeGraphItem.cpp
    editor/script/items/JZNodeGraphItem.h
	editor/script/items/JZNodeFunctionItem.cpp
	editor/script/items/JZNodeFunctionItem.h
    editor/script/items/JZNodeFlowItem.cpp
    editor/script/items/JZNodeFlowItem.h
    editor/script/items/JZNodeValueItem.cpp
    editor/script/items/JZNodeValueItem.h
    editor/script/items/JZNodeOperatorItem.cpp
    editor/script/items/JZNodeOperatorItem.h  
    editor/script/items/JZNodeSettingDialog.cpp
    editor/script/items/JZNodeSettingDialog.h
    editor/script/items/JZNodeDisplayItem.cpp
    editor/script/items/JZNodeDisplayItem.h
     
    widgets/JZAboutDialog.cpp
    widgets/JZAboutDialog.h    
    widgets/JZSearchTreeWidget.cpp
    widgets/JZSearchTreeWidget.h  
    widgets/JZDebugSettingDialog.cpp
    widgets/JZDebugSettingDialog.h
    widgets/JZDebugSettingDialog.ui
    
    remote/JZRemotePacket.cpp
    remote/JZRemotePacket.h
    remote/JZRemoteClient.cpp
    remote/JZRemoteClient.h
    
    #sample
    sample/Russian/Russian.cpp
    sample/Russian/Russian.h
    sample/VisionDemo/VisionDemo.cpp
    sample/VisionDemo/VisionDemo.h
    sample/SampleProject.cpp
    sample/SampleProject.h

    #communication            
    modules/communication/modbus/JZModbusConfigDialog.cpp
    modules/communication/modbus/JZModbusConfigDialog.h
    modules/communication/modbus/JZModbusConfigDialog.ui
    modules/communication/modbus/JZModbusSimulator.cpp
    modules/communication/modbus/JZModbusSimulator.h
    modules/communication/modbus/JZModbusTemplateDialog.cpp
    modules/communication/modbus/JZModbusTemplateDialog.h    
    modules/communication/JZModuleCommEditor.cpp
    modules/communication/JZModuleCommEditor.h
    
    #opencv    
    modules/opencv/JZModuleOpencvEditor.cpp
    modules/opencv/JZModuleOpencvEditor.h         
	
	#model
    modules/model/JZModuleModelEditor.cpp
    modules/model/JZModuleModelEditor.h
    
	#camera    
    modules/camera/JZModuleCameraEditor.h
    modules/camera/JZModuleCameraEditor.cpp       
    
    #vision
    modules/vision/JZModuleVisionEditor.h
    modules/vision/JZModuleVisionEditor.cpp
    modules/vision/vision.qrc
    
    #motion
    modules/motion/JZModuleMotionEditor.h
    modules/motion/JZModuleMotionEditor.cpp
)

add_library(JZNodeCore STATIC ${JZNodeCore_SOURCES})
target_link_libraries(JZNodeCore Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml    
    Qt${QT_VERSION_MAJOR}::UiTools    
    ${OpenCV_LIBRARIES})   

add_library(JZNodeEditorCore STATIC ${JZNodeEditorCore_SOURCES})
target_link_libraries(JZNodeEditorCore Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml    
    Qt${QT_VERSION_MAJOR}::UiTools    
	Qt${QT_VERSION_MAJOR}::Sql
    ${OpenCV_LIBRARIES})   
	
add_executable(JZRemoteServer remote/main.cpp
    remote/JZRemoteServer.cpp
    remote/JZRemoteServer.h
    remote/JZRemotePacket.cpp
    remote/JZRemotePacket.h)
target_link_libraries(JZRemoteServer JZNodeCore
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml    
    Qt${QT_VERSION_MAJOR}::UiTools
    ${OpenCV_LIBRARIES})
    
add_executable(JZNodeUiRuntime runtime/main.cpp)
target_link_libraries(JZNodeUiRuntime JZNodeCore
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml    
    Qt${QT_VERSION_MAJOR}::UiTools
    ${OpenCV_LIBRARIES})    
set_target_properties(JZNodeUiRuntime PROPERTIES
    WIN32_EXECUTABLE TRUE
)

add_subdirectory(tests)
add_subdirectory(applications)