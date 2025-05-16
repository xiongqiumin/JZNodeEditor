#include <QSplitter>
#include <QFileDialog>
#include "JZModuleVisionEditor.h"
#include "JZVisionNode.h"
#include "JZEditorGlobal.h"
#include "JZNodeEditorManager.h"
#include "modules/opencv/CvToQt.h"
#include "JZMotionNode.h"

//JZModuleMotionEditorInit
void JZModuleMotionEditorInit()
{
    auto inst = editorManager()->instance();
}