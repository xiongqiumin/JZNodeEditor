cmake_minimum_required(VERSION 3.5)

project(JZNodeTest)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS widgets xml)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS core gui network widgets uitools serialport test printsupport xml)
find_package(OpenCV 4 REQUIRED)

if (MSVC)
    add_compile_options(/execution-charset:utf-8)
    add_compile_options(/bigobj)
endif()

include_directories(
    ../../
    ../../script
    ${OpenCV_INCLUDE_DIRS})

set(NodePath ../..)
set(JZNodeTest_SOURCES
    main.cpp
    ${NodePath}/script/JZParamItem.cpp
    ${NodePath}/script/JZParamItem.h
    ${NodePath}/script/JZProject.cpp
    ${NodePath}/script/JZProject.h
    ${NodePath}/script/JZProjectItem.cpp
    ${NodePath}/script/JZProjectItem.h    
    ${NodePath}/script/JZProjectTemplate.cpp
    ${NodePath}/script/JZProjectTemplate.h
    ${NodePath}/script/JZScriptFile.cpp
    ${NodePath}/script/JZScriptFile.h
    ${NodePath}/script/JZScriptItem.cpp
    ${NodePath}/script/JZScriptItem.h
    ${NodePath}/script/JZClassItem.cpp
    ${NodePath}/script/JZClassItem.h
    ${NodePath}/script/JZProjectFile.cpp
    ${NodePath}/script/JZProjectFile.h
    ${NodePath}/script/JZUiItem.cpp
    ${NodePath}/script/JZUiItem.h
    ${NodePath}/script/JZContainer.cpp
    ${NodePath}/script/JZContainer.h
    ${NodePath}/script/JZEvent.cpp
    ${NodePath}/script/JZEvent.h
    ${NodePath}/script/JZExpressionSolver.cpp
    ${NodePath}/script/JZExpressionSolver.h
    ${NodePath}/script/JZModule.cpp
    ${NodePath}/script/JZModule.h
    ${NodePath}/script/JZNode.cpp
    ${NodePath}/script/JZNode.h
    ${NodePath}/script/JZNodeBind.cpp
    ${NodePath}/script/JZNodeBind.h
    ${NodePath}/script/JZNodeBuilder.cpp
    ${NodePath}/script/JZNodeBuilder.h
    ${NodePath}/script/JZNodeCompiler.cpp
    ${NodePath}/script/JZNodeCompiler.h
    ${NodePath}/script/JZNodeCppGenerater.cpp
    ${NodePath}/script/JZNodeCppGenerater.h
    ${NodePath}/script/JZNodeDebugClient.cpp
    ${NodePath}/script/JZNodeDebugClient.h
    ${NodePath}/script/JZNodeDebugPacket.cpp
    ${NodePath}/script/JZNodeDebugPacket.h
    ${NodePath}/script/JZNodeDebugServer.cpp
    ${NodePath}/script/JZNodeDebugServer.h
    ${NodePath}/script/JZNodeEngine.cpp
    ${NodePath}/script/JZNodeEngine.h
    ${NodePath}/script/JZNodeEnum.cpp
    ${NodePath}/script/JZNodeEnum.h
    ${NodePath}/script/JZNodeEvent.cpp
    ${NodePath}/script/JZNodeEvent.h
    ${NodePath}/script/JZNodeExpression.cpp
    ${NodePath}/script/JZNodeExpression.h
    ${NodePath}/script/JZNodeFactory.cpp
    ${NodePath}/script/JZNodeFactory.h
    ${NodePath}/script/JZNodeFunction.cpp
    ${NodePath}/script/JZNodeFunction.h
    ${NodePath}/script/JZNodeFunctionDefine.cpp
    ${NodePath}/script/JZNodeFunctionDefine.h
    ${NodePath}/script/JZNodeFunctionManager.cpp
    ${NodePath}/script/JZNodeFunctionManager.h
    ${NodePath}/script/JZScriptEnvironment.cpp
    ${NodePath}/script/JZScriptEnvironment.h
    ${NodePath}/script/JZNodeInit.cpp
    ${NodePath}/script/JZNodeInit.h
    ${NodePath}/script/JZNodeIR.cpp
    ${NodePath}/script/JZNodeIR.h
    ${NodePath}/script/JZNodeLangServer.cpp
    ${NodePath}/script/JZNodeLangServer.h
    ${NodePath}/script/JZNodeMeta.cpp
    ${NodePath}/script/JZNodeMeta.h
    ${NodePath}/script/JZNodeObject.cpp
    ${NodePath}/script/JZNodeObject.h
    ${NodePath}/script/JZNodeObjectParser.cpp
    ${NodePath}/script/JZNodeObjectParser.h
    ${NodePath}/script/JZNodeOperator.cpp
    ${NodePath}/script/JZNodeOperator.h
    ${NodePath}/script/JZNodePin.cpp
    ${NodePath}/script/JZNodePin.h
    ${NodePath}/script/JZNodeProgram.cpp
    ${NodePath}/script/JZNodeProgram.h
    ${NodePath}/script/JZNodeProgramDumper.cpp
    ${NodePath}/script/JZNodeProgramDumper.h
    ${NodePath}/script/JZNodeQtWrapper.cpp
    ${NodePath}/script/JZNodeQtWrapper.h
    ${NodePath}/script/JZNodeType.cpp
    ${NodePath}/script/JZNodeType.h
    ${NodePath}/script/JZNodeUtils.cpp
    ${NodePath}/script/JZNodeUtils.h
    ${NodePath}/script/JZNodeValue.cpp
    ${NodePath}/script/JZNodeValue.h
    ${NodePath}/script/JZNodeVariableBind.cpp
    ${NodePath}/script/JZNodeVariableBind.h
    ${NodePath}/script/JZNodeVM.cpp
    ${NodePath}/script/JZNodeVM.h
    ${NodePath}/script/JZRegExpHelp.cpp
    ${NodePath}/script/JZRegExpHelp.h
    ${NodePath}/script/JZScriptBuildInFunction.cpp
    ${NodePath}/script/JZScriptBuildInFunction.h
    ${NodePath}/3rd/JZCommon/jzNet/JZNetClient.cpp
    ${NodePath}/3rd/JZCommon/jzNet/JZNetClient.h
    ${NodePath}/3rd/JZCommon/jzNet/JZNetDataManager.cpp
    ${NodePath}/3rd/JZCommon/jzNet/JZNetDataManager.h
    ${NodePath}/3rd/JZCommon/jzNet/JZNetPack.cpp
    ${NodePath}/3rd/JZCommon/jzNet/JZNetPack.h
    ${NodePath}/3rd/JZCommon/jzNet/JZNetServer.cpp
    ${NodePath}/3rd/JZCommon/jzNet/JZNetServer.h
    ${NodePath}/3rd/JZCommon/jzCommon/JZLogManager.h
    ${NodePath}/3rd/JZCommon/jzCommon/JZLogManager.cpp
	${NodePath}/3rd/JZCommon/jzWidgets/JZImageView.h
    ${NodePath}/3rd/JZCommon/jzWidgets/JZImageView.cpp

    ${NodePath}/LogManager.cpp
    ${NodePath}/LogManager.h
    ${NodePath}/modules/camera/JZModuleCamera.h
    ${NodePath}/modules/camera/JZModuleCamera.cpp
    ${NodePath}/modules/camera/JZCamera.h
    ${NodePath}/modules/camera/JZCamera.cpp
    ${NodePath}/modules/camera/JZCameraFile.h
    ${NodePath}/modules/camera/JZCameraFile.cpp

    ${NodePath}/modules/opencv/JZModuleOpencv.h
    ${NodePath}/modules/opencv/JZModuleOpencv.cpp
    ${NodePath}/modules/opencv/CvMatAndQImage.h
    ${NodePath}/modules/opencv/CvMatAndQImage.cpp
    ${NodePath}/modules/opencv/CvToQt.h
    ${NodePath}/modules/opencv/CvToQt.cpp
    ${NodePath}/modules/opencv/JZYolo.h
    ${NodePath}/modules/opencv/JZYolo.cpp
	${NodePath}/modules/opencv/JZYoloView.h
	${NodePath}/modules/opencv/JZYoloView.cpp
)

add_executable(JZNodeTest ${JZNodeTest_SOURCES})
target_link_libraries(JZNodeTest Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network          
    Qt${QT_VERSION_MAJOR}::SerialPort      
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml
    ${OpenCV_LIBRARIES})