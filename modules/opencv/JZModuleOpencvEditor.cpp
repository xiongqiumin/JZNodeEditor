#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSplitter>
#include "JZModuleOpencvEditor.h"
#include "JZNodeParamDisplayWidget.h"
#include "CvToQt.h"
#include "JZNodeView.h"
#include "JZNodeEditorManager.h"
#include "JZEditorGlobal.h"

using namespace cv;


//JZModuleModelEditorInit
void JZModuleOpencvEditorInit()
{
    auto inst = editorManager()->instance();    
}