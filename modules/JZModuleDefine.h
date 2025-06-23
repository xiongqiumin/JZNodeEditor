#ifndef MODULE_DEFINE_H_
#define MODULE_DEFINE_H_

enum ModuleClass
{
    Module_CameraType = 16000,
    Module_ModbusType = 17000,
    Module_OpencvType = 18000,
    Module_ModelType  = 19000,
    Module_CommType   = 20000,
    Module_VisionType = 21000,
    Module_MotionType = 22000,
    Module_VisionAppType = 23000,
    Module_LogType = 24000,
};

enum ModuleNode
{
    Module_CameraNode = 1000,
    Module_ModbusNode = 1100,
    Module_OpencvNode = 1200,
    Module_ModelNode  = 1300,
    Module_CommNode   = 1400,
    Module_VisionNode = 1500,
    Module_MotionNode = 1600,
    Module_LogNode = 1600,
    Module_VisionAppNode = 1700,
};

enum ModuleProjectItem
{
    Module_visionItem = 1000,    
};

#endif