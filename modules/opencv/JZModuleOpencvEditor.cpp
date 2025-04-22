#include "JZModuleOpencvEditor.h"
#include "JZNodeParamDisplayWidget.h"

JZModuleOpencvEditor::JZModuleOpencvEditor()
{
}

JZModuleOpencvEditor::~JZModuleOpencvEditor()
{
}

/*
void JZModuleOpencvEditor::regist(JZScriptEnvironment *env)
{
    auto d_inst = JZNodeEditorManager::instance();

    JZNodeParamDelegate d_mat;
    d_mat.editType = Type_imageEdit;
    d_mat.createDisplay = CreateParamDisplayWidget<JZNodeImageDisplayWidget>;
    d_mat.createParam = createMat;    
    d_mat.pack = matPack;
    d_mat.unpack = matUnpack;
    d_inst->registDelegate(cls_mat.id(), d_mat);
}
*/