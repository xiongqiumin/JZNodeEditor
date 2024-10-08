cmake_minimum_required(VERSION 3.5)

project(JZNodeEditor)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS core gui network widgets uitools serialport test designer printsupport DesignerComponents xml)

include_directories(./
    ./widgets
    ./project
    ./script
    ./editor
    ./editor/script
    ./editor/ui
    ./3rd/jznet
    ./3rd/jzprofiler)

add_subdirectory(3rd)

set(JZNodeCore_SOURCES    
    JZNodeCore.cpp
    JZNodeCore.h
    JZNodeCoreDefine.h
    JZNodeEditor.qrc
    LogManager.cpp
    LogManager.h
    LogWidget.cpp
    LogWidget.h    
    mainwindow.cpp
    mainwindow.h
    UiCommon.cpp
    UiCommon.h    
    3rd/jznet/JZNetClient.cpp
    3rd/jznet/JZNetClient.h
    3rd/jznet/JZNetDataManager.cpp
    3rd/jznet/JZNetDataManager.h
    3rd/jznet/JZNetPack.cpp
    3rd/jznet/JZNetPack.h
    3rd/jznet/JZNetServer.cpp
    3rd/jznet/JZNetServer.h
    3rd/jzprofiler/JZTx.cpp
    3rd/jzprofiler/JZTx.h
    3rd/jzupdate/JZUpdateClient.cpp
    3rd/jzupdate/JZUpdateClient.h
    3rd/jzupdate/JZUpdateDialog.cpp
    3rd/jzupdate/JZUpdateDialog.h
    3rd/jzupdate/JZUpdatePack.cpp
    3rd/jzupdate/JZUpdatePack.h
    3rd/jzupdate/JZUpdateServer.cpp
    3rd/jzupdate/JZUpdateServer.h
    3rd/jzupdate/JZUpdateTest.cpp
    3rd/jzupdate/JZUpdateTest.h
    3rd/jzupdate/JZUpdateUtils.cpp
    3rd/jzupdate/JZUpdateUtils.h
    3rd/qcustomplot/JZPlotConfg.cpp
    3rd/qcustomplot/JZPlotConfg.h
    3rd/qcustomplot/qcustomplot.cpp
    3rd/qcustomplot/qcustomplot.h
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
    editor/script/JZNodeAutoRunThread.cpp
    editor/script/JZNodeAutoRunThread.h
    editor/script/JZNodeAutoRunWidget.cpp
    editor/script/JZNodeAutoRunWidget.h
    editor/script/JZNodeBaseItem.cpp
    editor/script/JZNodeBaseItem.h
    editor/script/JZNodeBreakPoint.cpp
    editor/script/JZNodeBreakPoint.h
    editor/script/JZNodeBuildThread.cpp
    editor/script/JZNodeBuildThread.h
    editor/script/JZNodeClassEditDialog.cpp
    editor/script/JZNodeClassEditDialog.h
    editor/script/JZNodeClassEditDialog.ui
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
    editor/script/JZNodeGraphItem.cpp
    editor/script/JZNodeGraphItem.h
    editor/script/JZNodeGroupEditDialog.cpp
    editor/script/JZNodeGroupEditDialog.h
    editor/script/JZNodeGroupEditDialog.ui
    editor/script/JZNodeGroupItem.cpp
    editor/script/JZNodeGroupItem.h
    editor/script/JZNodeLayout.cpp
    editor/script/JZNodeLayout.h
    editor/script/JZNodeLineItem.cpp
    editor/script/JZNodeLineItem.h
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
    editor/script/JZNodePinWidget.cpp
    editor/script/JZNodePinWidget.h
    editor/script/JZNodeParamEditWidget.cpp
    editor/script/JZNodeParamEditWidget.h
    editor/script/JZNodeParamDisplayWidget.cpp
    editor/script/JZNodeParamDisplayWidget.h    
    editor/script/JZNodePropertyBrowser.cpp
    editor/script/JZNodePropertyBrowser.h
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
    editor/script/JZNodeViewMap.cpp
    editor/script/JZNodeViewMap.h
    editor/script/JZNodeViewPanel.cpp
    editor/script/JZNodeViewPanel.h
    editor/script/JZNodeVirtualFuctionEditDialog.cpp
    editor/script/JZNodeVirtualFuctionEditDialog.h
    editor/script/JZNodeWatch.cpp
    editor/script/JZNodeWatch.h
    editor/script/JZProcess.cpp
    editor/script/JZProcess.h        
    editor/ui/formresizer.cpp
    editor/ui/formresizer.h
    editor/ui/JZDesignerEditor.cpp
    editor/ui/JZDesignerEditor.h
    editor/ui/JZDesignerFormWindow.cpp
    editor/ui/JZDesignerFormWindow.h
    editor/ui/JZDesinger.cpp
    editor/ui/JZDesinger.h
    editor/ui/JZMenuExtension.cpp
    editor/ui/JZMenuExtension.h
    editor/ui/JZNodeIntegration.cpp
    editor/ui/JZNodeIntegration.h
    editor/ui/JZUiEditor.cpp
    editor/ui/JZUiEditor.h
    editor/ui/JZWidgetCollectionInterface.cpp
    editor/ui/JZWidgetCollectionInterface.h
    editor/ui/sizehandlerect.cpp
    editor/ui/sizehandlerect.h
    editor/ui/widgethostconstants.h    
    modules/JZModuleNetwok.cpp
    modules/JZModuleNetwok.h
    project/JZClassItem.cpp
    project/JZClassItem.h
    project/JZParamItem.cpp
    project/JZParamItem.h
    project/JZProject.cpp
    project/JZProject.h
    project/JZProjectItem.cpp
    project/JZProjectItem.h
    project/JZProjectStream.cpp
    project/JZProjectStream.h
    project/JZProjectTemplate.cpp
    project/JZProjectTemplate.h
    project/JZScriptFile.cpp
    project/JZScriptFile.h
    project/JZScriptItem.cpp
    project/JZScriptItem.h
    project/JZUiFile.cpp
    project/JZUiFile.h
    script/JZContainer.cpp
    script/JZContainer.h
    script/JZEvent.cpp
    script/JZEvent.h
    script/JZExpressionSolver.cpp
    script/JZExpressionSolver.h
    script/JZModule.cpp
    script/JZModule.h
    script/JZNode.cpp
    script/JZNode.h
    script/JZNodeBind.cpp
    script/JZNodeBind.h
    script/JZNodeBuilder.cpp
    script/JZNodeBuilder.h
    script/JZNodeCompiler.cpp
    script/JZNodeCompiler.h
    script/JZNodeCppGenerater.cpp
    script/JZNodeCppGenerater.h
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
    script/JZNodeUiLoader.cpp
    script/JZNodeUiLoader.h
    script/JZNodeUtils.cpp
    script/JZNodeUtils.h
    script/JZNodeValue.cpp
    script/JZNodeValue.h
    script/JZNodeVariableBind.cpp
    script/JZNodeVariableBind.h
    script/JZNodeVM.cpp
    script/JZNodeVM.h
    script/JZRegExpHelp.cpp
    script/JZRegExpHelp.h
    script/JZScriptBuildInFunction.cpp
    script/JZScriptBuildInFunction.h
    script/angelscript/angelscript.h
    script/angelscript/as_config.h
    script/angelscript/as_parser.cpp
    script/angelscript/as_parser.h
    script/angelscript/as_scriptcode.cpp
    script/angelscript/as_scriptcode.h
    script/angelscript/as_scriptnode.cpp
    script/angelscript/as_scriptnode.h
    script/angelscript/as_texts.h
    script/angelscript/as_tojzscript.cpp
    script/angelscript/as_tojzscript.h
    script/angelscript/as_tokendef.h
    script/angelscript/as_tokenizer.cpp
    script/angelscript/as_tokenizer.h
    widgets/JZAboutDialog.cpp
    widgets/JZAboutDialog.h
    widgets/JZBaseDialog.cpp
    widgets/JZBaseDialog.h
    widgets/JZSearchTreeWidget.cpp
    widgets/JZSearchTreeWidget.h
    widgets/QImageLabel.cpp
    widgets/QImageLabel.h  
)


add_library(JZNodeCore SHARED ${JZNodeCore_SOURCES})
target_compile_definitions(JZNodeCore PRIVATE -DJZCORE_SDK_EXPORTS)
target_link_libraries(JZNodeCore PUBLIC 
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport)

target_link_libraries(JZNodeCore PRIVATE
    Qt${QT_VERSION_MAJOR}::UiTools      
    Qt${QT_VERSION_MAJOR}::Designer
    Qt${QT_VERSION_MAJOR}::DesignerComponents)

add_executable(JZNodeEditor main.cpp)
target_link_libraries(JZNodeEditor JZNodeCore)
set_target_properties(JZNodeEditor PROPERTIES    
    WIN32_EXECUTABLE TRUE
)

add_subdirectory(modules)
add_subdirectory(sample)
add_subdirectory(tests)