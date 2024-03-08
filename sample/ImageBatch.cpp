#include <QApplication>
#include <QDir>
#include "ImageBatch.h"
#include "JZNodeBuilder.h"
#include "JZNodeVM.h"
#include "UiCommon.h"
#include "JZNodeFunction.h"
#include "JZNodeValue.h"
#include "JZUiFile.h"
#include "JZNodeView.h"
#include "JZNodeUtils.h"

SampleImageBatch::SampleImageBatch()
{
    newProject("imagebatch");        

    JZUiFile *ui_file = dynamic_cast<JZUiFile*>(m_project.getItem("./mainwindow.ui"));
    ui_file->setXml(loadUi("ImageBatch.ui"));
    m_project.saveItem(ui_file);

    auto class_file = m_project.getClass("MainWindow");
    
    addProcessImage();
    addProcessDir();
    addEvent();
}

SampleImageBatch::~SampleImageBatch()
{

}

void SampleImageBatch::addEvent()
{
    auto flow_srcipt = m_project.getClass("MainWindow")->flow("ÊÂ¼þ");
    auto btn_meta = JZNodeObjectManager::instance()->meta("PushButton");
    
    for (int i = 0; i < 2; i++)
    {
        QString btn_name = (i == 0)? "btnSelInput":"btnSelOutput";
        QString line_name = (i == 0)? "this.lineInput":"lineOutput";

        JZNodeSingleEvent *func_clicked = new JZNodeSingleEvent();
        JZNodeFunction *func_open_dialog = new JZNodeFunction();
        JZNodeFunction *func_isEmpty = new JZNodeFunction();
        JZNodeBranch *branch = new JZNodeBranch();
        JZNodeParam *param_line = new JZNodeParam();
        JZNodeFunction *set_text = new JZNodeFunction();

        func_clicked->setSingle(btn_meta->single("clicked"));
        func_clicked->setVariable(btn_name);
        func_open_dialog->setFunction("FileDialog.getExistingDirectory");
        func_isEmpty->setFunction("String.isEmpty");
        param_line->setVariable(line_name);
        set_text->setFunction("LineEdit.setText");

        flow_srcipt->addNode(JZNodePtr(func_clicked));
        flow_srcipt->addNode(JZNodePtr(func_open_dialog));
        flow_srcipt->addNode(JZNodePtr(func_isEmpty));
        flow_srcipt->addNode(JZNodePtr(branch));
        flow_srcipt->addNode(JZNodePtr(param_line));
        flow_srcipt->addNode(JZNodePtr(set_text));

        flow_srcipt->addConnect(func_clicked->flowOutGemo(), func_open_dialog->flowInGemo());
        flow_srcipt->addConnect(func_open_dialog->flowOutGemo(), branch->flowInGemo());

        flow_srcipt->addConnect(branch->flowOutGemo(1), set_text->flowInGemo());
        flow_srcipt->addConnect(func_open_dialog->paramOutGemo(0), func_isEmpty->paramInGemo(0));
        flow_srcipt->addConnect(func_isEmpty->paramOutGemo(0), branch->paramInGemo(0));

        flow_srcipt->addConnect(param_line->paramOutGemo(0), set_text->paramInGemo(0));
        flow_srcipt->addConnect(func_open_dialog->paramOutGemo(0), set_text->paramInGemo(1));
    }

    JZNodeSingleEvent *deal_clicked = new JZNodeSingleEvent();
    JZNodeFunction *func_deal = new JZNodeFunction();
    JZNodeParam *param_lineInput = new JZNodeParam();
    JZNodeParam *param_lineOutput = new JZNodeParam();
    JZNodeOr *param_or_empty = new JZNodeOr();
    JZNodeOr *param_or_exist = new JZNodeOr();
    JZNodeIf *if_exist = new JZNodeIf();
    JZNodeIf *if_empty = new JZNodeIf();
    JZNodeFunction *lineInput_text = new JZNodeFunction();
    JZNodeFunction *lineOutput_text = new JZNodeFunction();
    JZNodeFunction *input_empty = new JZNodeFunction();
    JZNodeFunction *output_empty = new JZNodeFunction();
    JZNodeReturn *return_empty = new JZNodeReturn();
    JZNodeReturn *return_exist = new JZNodeReturn();
    JZNodeFunction *input_exist = new JZNodeFunction();
    JZNodeFunction *output_exist = new JZNodeFunction();

    deal_clicked->setSingle(btn_meta->single("clicked"));
    deal_clicked->setVariable("btnDeal");
    param_lineInput->setVariable("lineInput");
    param_lineOutput->setVariable("lineOutput");
    func_deal->setFunction("MainWindow.processDir");
    lineInput_text->setFunction("LineEdit.text");
    lineOutput_text->setFunction("LineEdit.text");
    input_empty->setFunction("String.isEmpty");
    output_empty->setFunction("String.isEmpty");
    input_exist->setFunction("File.exists");
    output_exist->setFunction("File.exists");
    flow_srcipt->addNode(JZNodePtr(deal_clicked));
    flow_srcipt->addNode(JZNodePtr(func_deal));    

    JZNodeThis *node_this = new JZNodeThis();
    JZNodeFunction *msg_empty = new JZNodeFunction();
    JZNodeFunction *msg_noExist = new JZNodeFunction();
    msg_empty->setFunction("MessageBox.information");
    msg_noExist->setFunction("MessageBox.information");
    msg_empty->setParamInValue(1, "\"input or output is empty\"");
    msg_noExist->setParamInValue(1, "\"input or output not exists\"");
    flow_srcipt->addNode(JZNodePtr(node_this));
    flow_srcipt->addNode(JZNodePtr(msg_empty));
    flow_srcipt->addNode(JZNodePtr(msg_noExist));
    flow_srcipt->addConnect(node_this->paramOutGemo(0), msg_empty->paramInGemo(0));
    flow_srcipt->addConnect(node_this->paramOutGemo(0), msg_noExist->paramInGemo(0));

    flow_srcipt->addNode(JZNodePtr(param_lineInput));
    flow_srcipt->addNode(JZNodePtr(param_lineOutput));
    flow_srcipt->addNode(JZNodePtr(lineInput_text));
    flow_srcipt->addNode(JZNodePtr(lineOutput_text));
    flow_srcipt->addConnect(param_lineInput->paramOutGemo(0), lineInput_text->paramInGemo(0));
    flow_srcipt->addConnect(param_lineOutput->paramOutGemo(0), lineOutput_text->paramInGemo(0));
    

    // if empty
    flow_srcipt->addNode(JZNodePtr(param_or_empty));
    flow_srcipt->addNode(JZNodePtr(if_empty));
    flow_srcipt->addNode(JZNodePtr(return_empty));
    flow_srcipt->addConnect(deal_clicked->flowOutGemo(), if_empty->flowInGemo());
    flow_srcipt->addConnect(param_or_empty->paramOutGemo(0), if_empty->paramInGemo(0));    
    flow_srcipt->addConnect(if_empty->subFlowOutGemo(0), msg_empty->flowInGemo());
    flow_srcipt->addConnect(msg_empty->flowOutGemo(0), return_empty->flowInGemo());
    flow_srcipt->addNode(JZNodePtr(input_empty));
    flow_srcipt->addNode(JZNodePtr(output_empty));
    flow_srcipt->addConnect(lineInput_text->paramOutGemo(0), input_empty->paramInGemo(0));
    flow_srcipt->addConnect(lineOutput_text->paramOutGemo(0), output_empty->paramInGemo(0));
    flow_srcipt->addConnect(input_empty->paramOutGemo(0), param_or_empty->paramInGemo(0));
    flow_srcipt->addConnect(output_empty->paramOutGemo(0), param_or_empty->paramInGemo(1));    
    
    // if exist
    JZNodeNot *not1 = new JZNodeNot();
    JZNodeNot *not2 = new JZNodeNot();
    flow_srcipt->addNode(JZNodePtr(param_or_exist));
    flow_srcipt->addNode(JZNodePtr(if_exist));
    flow_srcipt->addNode(JZNodePtr(return_exist));    
    flow_srcipt->addNode(JZNodePtr(not1));
    flow_srcipt->addNode(JZNodePtr(not2));
    flow_srcipt->addConnect(if_empty->flowOutGemo(), if_exist->flowInGemo());
    flow_srcipt->addConnect(if_exist->subFlowOutGemo(0), msg_noExist->flowInGemo());
    flow_srcipt->addConnect(msg_noExist->flowOutGemo(0), return_exist->flowInGemo());
    flow_srcipt->addConnect(param_or_exist->paramOutGemo(0), if_exist->paramInGemo(0));
    flow_srcipt->addNode(JZNodePtr(input_exist));
    flow_srcipt->addNode(JZNodePtr(output_exist));
    flow_srcipt->addConnect(lineInput_text->paramOutGemo(0), input_exist->paramInGemo(0));
    flow_srcipt->addConnect(lineOutput_text->paramOutGemo(0), output_exist->paramInGemo(0));
    flow_srcipt->addConnect(input_exist->paramOutGemo(0), not1->paramInGemo(0));
    flow_srcipt->addConnect(output_exist->paramOutGemo(0), not2->paramInGemo(0));
    flow_srcipt->addConnect(not1->paramOutGemo(0), param_or_exist->paramInGemo(0));
    flow_srcipt->addConnect(not2->paramOutGemo(0), param_or_exist->paramInGemo(1));

    JZNodeParam *param_lineInput2 = new JZNodeParam();
    JZNodeParam *param_lineOutput2 = new JZNodeParam();
    JZNodeFunction *lineInput_text2 = new JZNodeFunction();
    JZNodeFunction *lineOutput_text2 = new JZNodeFunction();
    flow_srcipt->addNode(JZNodePtr(param_lineInput2));
    flow_srcipt->addNode(JZNodePtr(param_lineOutput2));
    flow_srcipt->addNode(JZNodePtr(lineInput_text2));
    flow_srcipt->addNode(JZNodePtr(lineOutput_text2));
    param_lineInput2->setVariable("lineInput");
    param_lineOutput2->setVariable("lineOutput");    
    lineInput_text2->setFunction("LineEdit.text");
    lineOutput_text2->setFunction("LineEdit.text");
    flow_srcipt->addConnect(param_lineInput2->paramOutGemo(0), lineInput_text2->paramInGemo(0));
    flow_srcipt->addConnect(param_lineOutput2->paramOutGemo(0), lineOutput_text2->paramInGemo(0));
    flow_srcipt->addConnect(lineInput_text2->paramOutGemo(0), func_deal->paramInGemo(1));
    flow_srcipt->addConnect(lineOutput_text2->paramOutGemo(0), func_deal->paramInGemo(2));
    flow_srcipt->addConnect(if_exist->flowOutGemo(), func_deal->flowInGemo());

    JZNodeSingleEvent *exit_clicked = new JZNodeSingleEvent();
    JZNodeFunction *func_exit = new JZNodeFunction();
    exit_clicked->setSingle(btn_meta->single("clicked"));
    exit_clicked->setVariable("btnExit");
    func_exit->setFunction("MainWindow.close");
    flow_srcipt->addNode(JZNodePtr(exit_clicked));
    flow_srcipt->addNode(JZNodePtr(func_exit));
    flow_srcipt->addConnect(exit_clicked->flowOutGemo(), func_exit->flowInGemo());
}

void SampleImageBatch::addProcessImage()
{
    auto class_file = m_project.getClass("MainWindow");

    JZFunctionDefine define;
    define.name = "processImage";
    define.className = "MainWindow";
    define.paramIn.push_back(JZParamDefine("this", "MainWindow"));
    define.paramIn.push_back(JZParamDefine("input", "Image"));
    define.paramOut.push_back(JZParamDefine("output", "Image"));
    define.isFlowFunction = true;
    auto script = class_file->addMemberFunction(define);
    auto start = script->getNode(0);

    JZNodeParam *input = new JZNodeParam();
    JZNodeReturn *ret = new JZNodeReturn();
    input->setVariable("input");

    ret->setFunction("MainWindow.processImage");
    script->addNode(JZNodePtr(input));
    script->addNode(JZNodePtr(ret));

    script->addConnect(start->flowOutGemo(), ret->flowInGemo());
    script->addConnect(input->paramOutGemo(0), ret->paramInGemo(0));
}

void SampleImageBatch::addProcessDir()
{
    auto class_file = m_project.getClass("MainWindow");

    JZFunctionDefine define_dir;
    define_dir.name = "processDir";
    define_dir.className = "MainWindow";
    define_dir.paramIn.push_back(JZParamDefine("this", "MainWindow"));
    define_dir.paramIn.push_back(JZParamDefine("inputDir", "String"));
    define_dir.paramIn.push_back(JZParamDefine("outputDir", "String"));
    define_dir.isFlowFunction = true;
    auto script = class_file->addMemberFunction(define_dir);
    script->addLocalVariable(JZParamDefine("list", "StringList"));
    script->addLocalVariable(JZParamDefine("in", "String"));
    script->addLocalVariable(JZParamDefine("out", "String"));    

    JZNodeParam *in_dir = new JZNodeParam();
    JZNodeParam *out_dir = new JZNodeParam();
    JZNodeParam *get_in_file = new JZNodeParam();
    JZNodeParam *get_out_file = new JZNodeParam();
    JZNodeSetParam *set_in_file = new JZNodeSetParam();
    JZNodeSetParam *set_out_file = new JZNodeSetParam();
    JZNodeParam *get_list = new JZNodeParam();
    JZNodeSetParam *set_list = new JZNodeSetParam();
    JZNodeFunction *create_dir = new JZNodeFunction();
    JZNodeFunction *entry_info = new JZNodeFunction();
    JZNodeFunction *fileinfo_isFile = new JZNodeFunction();
    JZNodeFunction *file_process = new JZNodeFunction();
    JZNodeFunction *dir_process = new JZNodeFunction();
    JZNodeBranch *branch = new JZNodeBranch();
    JZNodeFunction *image_load = new JZNodeFunction();
    JZNodeFunction *image_save = new JZNodeFunction();
    JZNodeFor *node_for = new JZNodeFor();
    JZNodeFunction *list_size = new JZNodeFunction();    
    JZNodeStringAdd *op_add_in = new JZNodeStringAdd();
    JZNodeStringAdd *op_add_out = new JZNodeStringAdd();
    JZNodeFunction *list_at = new JZNodeFunction();
    JZNodeFunction *fileinfo_create = new JZNodeFunction();

    auto node_start = script->getNode(0);
    create_dir->setFunction("Dir.create");
    entry_info->setFunction("Dir.entryList");
    fileinfo_isFile->setFunction("FileInfo.isFile");
    file_process->setFunction("MainWindow.processImage");
    dir_process->setFunction("MainWindow.processDir");
    image_load->setFunction("Image.create");
    image_save->setFunction("Image.save");    
    list_at->setFunction("StringList.get");
    list_size->setFunction("StringList.size");
    fileinfo_create->setFunction("FileInfo.create");

    in_dir->setVariable("inputDir");
    out_dir->setVariable("outputDir");
    get_in_file->setVariable("in");
    get_out_file->setVariable("out");
    set_in_file->setVariable("in");
    set_out_file->setVariable("out");
    set_list->setVariable("list");
    get_list->setVariable("list");

    script->addNode(JZNodePtr(in_dir));
    script->addNode(JZNodePtr(out_dir));
    script->addNode(JZNodePtr(set_in_file));
    script->addNode(JZNodePtr(set_out_file));
    script->addNode(JZNodePtr(get_in_file));
    script->addNode(JZNodePtr(get_out_file));
    script->addNode(JZNodePtr(get_list));
    script->addNode(JZNodePtr(set_list));
    script->addNode(JZNodePtr(create_dir));
    script->addNode(JZNodePtr(entry_info));
    script->addNode(JZNodePtr(fileinfo_isFile));
    script->addNode(JZNodePtr(file_process));
    script->addNode(JZNodePtr(dir_process));
    script->addNode(JZNodePtr(branch));
    script->addNode(JZNodePtr(image_load));
    script->addNode(JZNodePtr(image_save));
    script->addNode(JZNodePtr(node_for));
    script->addNode(JZNodePtr(list_size));    
    script->addNode(JZNodePtr(op_add_in));
    script->addNode(JZNodePtr(op_add_out));
    script->addNode(JZNodePtr(list_at));
    script->addNode(JZNodePtr(fileinfo_create));

    JZNodeFlag *dir_flag = new JZNodeFlag();
    dir_flag->setFlag("Dir::Filters");
    dir_flag->setValue(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    script->addNode(JZNodePtr(dir_flag));

    script->addConnect(in_dir->paramOutGemo(0), create_dir->paramInGemo(0));
    script->addConnect(node_start->flowOutGemo(0), set_list->flowInGemo());    
    script->addConnect(create_dir->paramOutGemo(0), entry_info->paramInGemo(0));
    entry_info->setParamInValue(1, JZNodeType::storgeString("*.jpg;*.png;*.bmp"));
    script->addConnect(dir_flag->paramOutGemo(0), entry_info->paramInGemo(2));
    script->addConnect(entry_info->paramOutGemo(0), set_list->paramInGemo(1));
    script->addConnect(set_list->flowOutGemo(0), node_for->flowInGemo());
    
    script->addConnect(node_for->paramOutGemo(0), list_at->paramInGemo(1));
    // in = in_dir + "/" + list[i]
    script->addConnect(node_for->subFlowOutGemo(0), set_in_file->flowInGemo());
    script->addConnect(get_list->paramOutGemo(0), list_size->paramInGemo(0));
    script->addConnect(list_size->paramOutGemo(0), node_for->paramInGemo(2));
    op_add_in->addInput();
    script->addConnect(in_dir->paramOutGemo(0), op_add_in->paramInGemo(0));
    op_add_in->setParamInValue(1, JZNodeType::storgeString("/"));
    script->addConnect(get_list->paramOutGemo(0), list_at->paramInGemo(0));
    script->addConnect(list_at->paramOutGemo(0), op_add_in->paramInGemo(2));    
    script->addConnect(op_add_in->paramOutGemo(0), set_in_file->paramInGemo(1));
    
    // out = out_dir + "/" + list[i]
    script->addConnect(set_in_file->flowOutGemo(0), set_out_file->flowInGemo());
    op_add_out->addInput();
    script->addConnect(out_dir->paramOutGemo(0), op_add_out->paramInGemo(0));
    op_add_out->setParamInValue(1, JZNodeType::storgeString("/"));
    script->addConnect(list_at->paramOutGemo(0), op_add_out->paramInGemo(2));    
    script->addConnect(op_add_out->paramOutGemo(0), set_out_file->paramInGemo(1));

    script->addConnect(set_out_file->flowOutGemo(0), branch->flowInGemo());
    script->addConnect(fileinfo_isFile->paramOutGemo(0), branch->paramInGemo(0));
    script->addConnect(get_in_file->paramOutGemo(0), fileinfo_create->paramInGemo(0));
    script->addConnect(fileinfo_create->paramOutGemo(0), fileinfo_isFile->paramInGemo(0));

    script->addConnect(branch->flowOutGemo(0), file_process->flowInGemo());
    script->addConnect(get_in_file->paramOutGemo(0), image_load->paramInGemo(0));
    script->addConnect(image_load->paramOutGemo(0), file_process->paramInGemo(1));    
    script->addConnect(file_process->flowOutGemo(0), image_save->flowInGemo());
    script->addConnect(file_process->paramOutGemo(0), image_save->paramInGemo(0));
    script->addConnect(get_out_file->paramOutGemo(0), image_save->paramInGemo(1));

    script->addConnect(branch->flowOutGemo(1), dir_process->flowInGemo());
    script->addConnect(get_in_file->paramOutGemo(0), dir_process->paramInGemo(1));
    script->addConnect(get_out_file->paramOutGemo(0), dir_process->paramInGemo(2));
}