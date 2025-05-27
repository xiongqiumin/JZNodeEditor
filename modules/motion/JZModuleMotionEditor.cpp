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

    inst->registLogicNode(Node_MotionInit, "运动控制");
    inst->registLogicNode(Node_MotionZero, "运动控制");
    inst->registLogicNode(Node_MotionMove, "运动控制");    
}