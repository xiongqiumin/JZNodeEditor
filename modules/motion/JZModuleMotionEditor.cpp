#include <QSplitter>
#include <QFileDialog>
#include "JZModuleMotionEditor.h"
#include "JZEditorGlobal.h"
#include "JZNodeEditorManager.h"
#include "modules/opencv/CvToQt.h"
#include "JZMotionNode.h"

//JZModuleMotionEditorInit
void JZModuleMotionEditorInit()
{
    auto inst = editorManager()->instance();

    inst->registLogicNode(Node_MotionInit, "运控初始化");
    inst->registLogicNode(Node_MotionZero, "运控归零");
    inst->registLogicNode(Node_MotionMove, "运控移动");    
}