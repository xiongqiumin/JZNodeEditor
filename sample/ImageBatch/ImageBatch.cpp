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
    addProcessGetImage();
    addProcessDir();    
    addEvent();
}

SampleImageBatch::~SampleImageBatch()
{

}

void SampleImageBatch::addEvent()
{
    auto flow_srcipt = m_project.getClass("MainWindow")->flow("�¼�");
    auto btn_meta = JZNodeObjectManager::instance()->meta("PushButton");
 /*   
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
        func_isEmpty->setFunction("string.isEmpty");
        param_line->setVariable(line_name);
        set_text->setFunction("LineEdit.setText");

        flow_srcipt->addNode(func_clicked);
        flow_srcipt->addNode(func_open_dialog);
        flow_srcipt->addNode(func_isEmpty);
        flow_srcipt->addNode(branch);
        flow_srcipt->addNode(param_line);
        flow_srcipt->addNode(set_text);

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
    input_empty->setFunction("string.isEmpty");
    output_empty->setFunction("string.isEmpty");
    input_exist->setFunction("File.exists");
    output_exist->setFunction("File.exists");
    flow_srcipt->addNode(deal_clicked);
    flow_srcipt->addNode(func_deal);    

    JZNodeThis *node_this = new JZNodeThis();
    JZNodeFunction *msg_empty = new JZNodeFunction();
    JZNodeFunction *msg_noExist = new JZNodeFunction();
    msg_empty->setFunction("MessageBox.information");
    msg_noExist->setFunction("MessageBox.information");
    msg_empty->setParamInValue(1, "\"input or output is empty\"");
    msg_noExist->setParamInValue(1, "\"input or output not exists\"");
    flow_srcipt->addNode(node_this);
    flow_srcipt->addNode(msg_empty);
    flow_srcipt->addNode(msg_noExist);
    flow_srcipt->addConnect(node_this->paramOutGemo(0), msg_empty->paramInGemo(0));
    flow_srcipt->addConnect(node_this->paramOutGemo(0), msg_noExist->paramInGemo(0));

    flow_srcipt->addNode(param_lineInput);
    flow_srcipt->addNode(param_lineOutput);
    flow_srcipt->addNode(lineInput_text);
    flow_srcipt->addNode(lineOutput_text);
    flow_srcipt->addConnect(param_lineInput->paramOutGemo(0), lineInput_text->paramInGemo(0));
    flow_srcipt->addConnect(param_lineOutput->paramOutGemo(0), lineOutput_text->paramInGemo(0));
    

    // if empty
    flow_srcipt->addNode(param_or_empty);
    flow_srcipt->addNode(if_empty);
    flow_srcipt->addNode(return_empty);
    flow_srcipt->addConnect(deal_clicked->flowOutGemo(), if_empty->flowInGemo());
    flow_srcipt->addConnect(param_or_empty->paramOutGemo(0), if_empty->paramInGemo(0));    
    flow_srcipt->addConnect(if_empty->subFlowOutGemo(0), msg_empty->flowInGemo());
    flow_srcipt->addConnect(msg_empty->flowOutGemo(0), return_empty->flowInGemo());
    flow_srcipt->addNode(input_empty);
    flow_srcipt->addNode(output_empty);
    flow_srcipt->addConnect(lineInput_text->paramOutGemo(0), input_empty->paramInGemo(0));
    flow_srcipt->addConnect(lineOutput_text->paramOutGemo(0), output_empty->paramInGemo(0));
    flow_srcipt->addConnect(input_empty->paramOutGemo(0), param_or_empty->paramInGemo(0));
    flow_srcipt->addConnect(output_empty->paramOutGemo(0), param_or_empty->paramInGemo(1));    
    
    // if exist
    JZNodeNot *not1 = new JZNodeNot();
    JZNodeNot *not2 = new JZNodeNot();
    flow_srcipt->addNode(param_or_exist);
    flow_srcipt->addNode(if_exist);
    flow_srcipt->addNode(return_exist);    
    flow_srcipt->addNode(not1);
    flow_srcipt->addNode(not2);
    flow_srcipt->addConnect(if_empty->flowOutGemo(), if_exist->flowInGemo());
    flow_srcipt->addConnect(if_exist->subFlowOutGemo(0), msg_noExist->flowInGemo());
    flow_srcipt->addConnect(msg_noExist->flowOutGemo(0), return_exist->flowInGemo());
    flow_srcipt->addConnect(param_or_exist->paramOutGemo(0), if_exist->paramInGemo(0));
    flow_srcipt->addNode(input_exist);
    flow_srcipt->addNode(output_exist);
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
    flow_srcipt->addNode(param_lineInput2);
    flow_srcipt->addNode(param_lineOutput2);
    flow_srcipt->addNode(lineInput_text2);
    flow_srcipt->addNode(lineOutput_text2);
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
    flow_srcipt->addNode(exit_clicked);
    flow_srcipt->addNode(func_exit);
    flow_srcipt->addConnect(exit_clicked->flowOutGemo(), func_exit->flowInGemo());
*/
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

    script->addNode(input);
    script->addNode(ret);

    script->addConnect(start->flowOutGemo(), ret->flowInGemo());
    script->addConnect(input->paramOutGemo(0), ret->paramInGemo(0));
}

void SampleImageBatch::addProcessGetImage()
{
    auto class_file = m_project.getClass("MainWindow");

    JZFunctionDefine define_dir;
    define_dir.name = "walkDir";
    define_dir.className = "MainWindow";
    define_dir.isFlowFunction = true;
    define_dir.paramIn.push_back(JZParamDefine("this", "MainWindow"));
    define_dir.paramIn.push_back(JZParamDefine("inputDir", Type_string));
    define_dir.paramIn.push_back(JZParamDefine("outList","StringList"));
    auto script = class_file->addMemberFunction(define_dir);    
    script->addLocalVariable(JZParamDefine("list", "StringList"));
    script->addLocalVariable(JZParamDefine("in", Type_string));

    auto node_start = script->getNode(0);
    JZNodeParam *in_dir = new JZNodeParam();    
    JZNodeParam *get_in_file = new JZNodeParam();    
    JZNodeSetParam *set_in_file = new JZNodeSetParam();    
    JZNodeParam *get_list = new JZNodeParam();
    JZNodeSetParam *set_list = new JZNodeSetParam();
    JZNodeFunction *create_dir = new JZNodeFunction();
    JZNodeFunction *entry_info = new JZNodeFunction();
    JZNodeFunction *fileinfo_isFile = new JZNodeFunction();
    JZNodeFunction *outlist_push = new JZNodeFunction();
    JZNodeFunction *dir_process = new JZNodeFunction();
    JZNodeBranch *branch = new JZNodeBranch();    
    JZNodeFor *node_for = new JZNodeFor();
    JZNodeFunction *list_size = new JZNodeFunction();
    JZNodeStringAdd *op_add_in = new JZNodeStringAdd();    
    JZNodeFunction *list_at = new JZNodeFunction();
    JZNodeFunction *fileinfo_create = new JZNodeFunction();
    JZNodeParam *out_list = new JZNodeParam();    
    
    create_dir->setFunction("Dir.create");
    entry_info->setFunction("Dir.entryList");
    fileinfo_isFile->setFunction("FileInfo.isFile");
    outlist_push->setFunction("StringList.push_back");
    dir_process->setFunction("MainWindow.walkDir");    
    list_at->setFunction("StringList.get");
    list_size->setFunction("StringList.size");
    fileinfo_create->setFunction("FileInfo.create");
    out_list->setVariable("outList");

    in_dir->setVariable("inputDir");    
    get_in_file->setVariable("in");    
    set_in_file->setVariable("in");    
    set_list->setVariable("list");
    get_list->setVariable("list");

    script->addNode(in_dir);    
    script->addNode(set_in_file);   
    script->addNode(get_in_file);    
    script->addNode(get_list);
    script->addNode(set_list);
    script->addNode(create_dir);
    script->addNode(entry_info);
    script->addNode(fileinfo_isFile);    
    script->addNode(dir_process);
    script->addNode(branch);    
    script->addNode(node_for);
    script->addNode(list_size);
    script->addNode(op_add_in);    
    script->addNode(list_at);
    script->addNode(fileinfo_create);
    script->addNode(out_list);
    script->addNode(outlist_push);

    JZNodeFlag *dir_flag = new JZNodeFlag();
    dir_flag->setFlag("Dir::Filters");
    dir_flag->setValue(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    script->addNode(dir_flag);

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

    script->addConnect(set_in_file->flowOutGemo(0), branch->flowInGemo());    
    script->addConnect(fileinfo_isFile->paramOutGemo(0), branch->paramInGemo(0));
    script->addConnect(get_in_file->paramOutGemo(0), fileinfo_create->paramInGemo(0));
    script->addConnect(fileinfo_create->paramOutGemo(0), fileinfo_isFile->paramInGemo(0));

    script->addConnect(branch->flowOutGemo(0), outlist_push->flowInGemo());
    script->addConnect(out_list->paramOutGemo(0), outlist_push->paramInGemo(0));
    script->addConnect(get_in_file->paramOutGemo(0), outlist_push->paramInGemo(1));
    

    script->addConnect(branch->flowOutGemo(1), dir_process->flowInGemo());
    script->addConnect(get_in_file->paramOutGemo(0), dir_process->paramInGemo(1));
    script->addConnect(out_list->paramOutGemo(0), dir_process->paramInGemo(2));
}

void SampleImageBatch::addProcessDir()
{
    auto class_file = m_project.getClass("MainWindow");

    JZFunctionDefine define_dir;
    define_dir.name = "processDir";
    define_dir.className = "MainWindow";
    define_dir.paramIn.push_back(JZParamDefine("this", "MainWindow"));
    define_dir.paramIn.push_back(JZParamDefine("inputDir", Type_string));
    define_dir.paramIn.push_back(JZParamDefine("outputDir", Type_string));
    define_dir.isFlowFunction = true;
    auto script = class_file->addMemberFunction(define_dir);
    script->addLocalVariable(JZParamDefine("list", "StringList"));
    script->addLocalVariable(JZParamDefine("in", Type_string));
    script->addLocalVariable(JZParamDefine("out", Type_string));    
    script->addLocalVariable(JZParamDefine("out_dir", Type_string));
    script->addLocalVariable(JZParamDefine("dir", "Dir"));
    script->addLocalVariable(JZParamDefine("progress", "ProgressDialog"));

    JZNodeParam *in_dir = new JZNodeParam();
    JZNodeParam *out_dir = new JZNodeParam();
    JZNodeParam *get_in_file = new JZNodeParam();
    JZNodeParam *get_out_file = new JZNodeParam();
    JZNodeSetParam *set_in_file = new JZNodeSetParam();
    JZNodeSetParam *set_out_file = new JZNodeSetParam();
    JZNodeParam *get_list = new JZNodeParam();
    JZNodeSetParam *set_list = new JZNodeSetParam();            
    JZNodeFunction *list_create = new JZNodeFunction();
    JZNodeFunction *file_process = new JZNodeFunction();    
    JZNodeFunction *image_load = new JZNodeFunction();
    JZNodeFunction *image_save = new JZNodeFunction();
    JZNodeFor *node_for = new JZNodeFor();
    JZNodeFunction *list_size = new JZNodeFunction();        
    JZNodeFunction *list_at = new JZNodeFunction();    

    auto node_start = script->getNode(0);
    file_process->setFunction("MainWindow.processImage");    
    image_load->setFunction("Image.create");
    image_save->setFunction("Image.save");    
    list_at->setFunction("StringList.get");
    list_size->setFunction("StringList.size");    
    list_create->setFunction("StringList.create");
    
    in_dir->setVariable("inputDir");
    out_dir->setVariable("outputDir");

    set_in_file->setVariable("in");
    get_in_file->setVariable("in");        
    set_out_file->setVariable("out");
    get_out_file->setVariable("out");
    set_list->setVariable("list");
    get_list->setVariable("list");

    script->addNode(in_dir);
    script->addNode(out_dir);
    script->addNode(set_in_file);
    script->addNode(set_out_file);
    script->addNode(get_in_file);
    script->addNode(get_out_file);
    script->addNode(get_list);
    script->addNode(set_list);
    
    script->addNode(file_process);    
    script->addNode(image_load);
    script->addNode(image_save);
    script->addNode(node_for);
    script->addNode(list_size);       
    script->addNode(list_at);    
    
    JZNodeFunction *str_replace = new JZNodeFunction();
    JZNodeFunction *walk_dir = new JZNodeFunction();    
    walk_dir->setFunction("MainWindow.walkDir");
    str_replace->setFunction("string.replace");
    script->addNode(walk_dir);
    script->addNode(str_replace);
    script->addNode(list_create);
    
    script->addConnect(node_start->flowOutGemo(0), set_list->flowInGemo());        
    script->addConnect(list_create->paramOutGemo(0), set_list->paramInGemo(1));

    script->addConnect(set_list->flowOutGemo(0), walk_dir->flowInGemo());
    script->addConnect(in_dir->paramOutGemo(0), walk_dir->paramInGemo(1));
    script->addConnect(get_list->paramOutGemo(0), walk_dir->paramInGemo(2));
        
    JZNodeParam *pro_dlg = new JZNodeParam();
    JZNodeSetParam *set_pro_dlg = new JZNodeSetParam();
    JZNodeFunction *pro_dlg_create = new JZNodeFunction();
    JZNodeFunction *pro_dlg_set_label = new JZNodeFunction();
    JZNodeFunction *pro_dlg_set_value = new JZNodeFunction();
    JZNodeFunction *pro_dlg_set_range = new JZNodeFunction();
    JZNodeFunction *pro_dlg_cancel = new JZNodeFunction();
    JZNodeIf *if_cancel = new JZNodeIf();
    JZNodeReturn *node_ret = new JZNodeReturn();
    pro_dlg->setVariable("progress");
    set_pro_dlg->setVariable("progress");
    pro_dlg_create->setFunction("ProgressDialog.create");
    pro_dlg_set_label->setFunction("ProgressDialog.setLabelText");
    pro_dlg_set_value->setFunction("ProgressDialog.setValue");
    pro_dlg_set_range->setFunction("ProgressDialog.setRange");
    pro_dlg_cancel->setFunction("ProgressDialog.wasCanceled");
    script->addNode(pro_dlg);
    script->addNode(set_pro_dlg);
    script->addNode(pro_dlg_create);
    script->addNode(pro_dlg_set_label);
    script->addNode(pro_dlg_set_value);
    script->addNode(pro_dlg_set_range);
    script->addNode(pro_dlg_cancel);
    script->addNode(if_cancel);
    script->addNode(node_ret);

    script->addConnect(walk_dir->flowOutGemo(0), set_pro_dlg->flowInGemo());
    script->addConnect(pro_dlg_create->paramOutGemo(0), set_pro_dlg->paramInGemo(1));
    script->addConnect(set_pro_dlg->flowOutGemo(0), pro_dlg_set_range->flowInGemo());

    script->addConnect(pro_dlg->paramOutGemo(0), pro_dlg_set_range->paramInGemo(0));
    pro_dlg_set_range->setParamInValue(1, "0");    
    script->addConnect(list_size->paramOutGemo(0), pro_dlg_set_range->paramInGemo(2));
    script->addConnect(pro_dlg_set_range->flowOutGemo(0), node_for->flowInGemo());
    script->addConnect(get_list->paramOutGemo(0), list_at->paramInGemo(0));
    script->addConnect(node_for->paramOutGemo(0), list_at->paramInGemo(1));        
    script->addConnect(get_list->paramOutGemo(0), list_size->paramInGemo(0));
    script->addConnect(list_size->paramOutGemo(0), node_for->paramInGemo(2));

    script->addConnect(node_for->subFlowOutGemo(0), if_cancel->flowInGemo());
    script->addConnect(pro_dlg_cancel->paramOutGemo(0), if_cancel->paramInGemo(0));
    script->addConnect(pro_dlg->paramOutGemo(0), pro_dlg_cancel->paramInGemo(0));
    
    script->addConnect(if_cancel->subFlowOutGemo(0), node_ret->flowInGemo());

    script->addConnect(if_cancel->flowOutGemo(0), pro_dlg_set_label->flowInGemo());
    script->addConnect(pro_dlg->paramOutGemo(0), pro_dlg_set_label->paramInGemo(0));
    script->addConnect(list_at->paramOutGemo(0), pro_dlg_set_label->paramInGemo(1));

    script->addConnect(pro_dlg_set_label->flowOutGemo(0), pro_dlg_set_value->flowInGemo());
    script->addConnect(pro_dlg->paramOutGemo(0), pro_dlg_set_value->paramInGemo(0));
    script->addConnect(node_for->paramOutGemo(0), pro_dlg_set_value->paramInGemo(1));

    script->addConnect(pro_dlg_set_value->flowOutGemo(0),  set_in_file->flowInGemo());
    script->addConnect(list_at->paramOutGemo(0), set_in_file->paramInGemo(1));

    script->addConnect(set_in_file->flowOutGemo(0), set_out_file->flowInGemo());
    script->addConnect(get_in_file->paramOutGemo(0), str_replace->paramInGemo(0));
    script->addConnect(in_dir->paramOutGemo(0), str_replace->paramInGemo(1));
    script->addConnect(out_dir->paramOutGemo(0), str_replace->paramInGemo(2));
    script->addConnect(str_replace->paramOutGemo(0), set_out_file->paramInGemo(1));

    JZNodeParam *out_path = new JZNodeParam();
    JZNodeSetParam *set_out_path = new JZNodeSetParam();
    JZNodeSetParam *set_dir = new JZNodeSetParam();
    JZNodeFunction *dir_create = new JZNodeFunction();
    JZNodeFunction *dir_isExists = new JZNodeFunction();
    JZNodeFunction *dir_makepath = new JZNodeFunction();
    JZNodeFunction *fileInfo_create = new JZNodeFunction();
    JZNodeFunction *fileInfo_path = new JZNodeFunction();
    JZNodeIf *if_exist = new JZNodeIf();
    JZNodeNot *if_not_exist = new JZNodeNot();
    out_path->setVariable("out_dir");
    set_out_path->setVariable("out_dir");
    set_dir->setVariable("dir");
    dir_create->setFunction("Dir.create");
    dir_isExists->setFunction("Dir.exists");
    dir_makepath->setFunction("Dir.mkpath");
    fileInfo_create->setFunction("FileInfo.create");
    fileInfo_path->setFunction("FileInfo.path");

    script->addNode(fileInfo_create);
    script->addNode(dir_create);
    script->addNode(dir_isExists);
    script->addNode(dir_makepath);
    script->addNode(set_dir);    
    script->addNode(out_path);
    script->addNode(set_out_path);
    script->addNode(if_exist);
    script->addNode(if_not_exist);
    script->addNode(fileInfo_path);

    script->addConnect(set_out_file->flowOutGemo(0), set_out_path->flowInGemo());
    script->addConnect(fileInfo_path->paramOutGemo(0), set_out_path->paramInGemo(1));     
    script->addConnect(fileInfo_create->paramOutGemo(0), fileInfo_path->paramInGemo(0));
    script->addConnect(set_out_file->paramOutGemo(0), fileInfo_create->paramInGemo(0));
        
    script->addConnect(set_out_path->flowOutGemo(0), set_dir->flowInGemo());
    script->addConnect(dir_create->paramOutGemo(0), set_dir->paramInGemo(1));
    dir_create->setParamInValue(0, JZNodeType::storgeString(""));

    script->addConnect(set_dir->flowOutGemo(0), if_exist->flowInGemo());
    script->addConnect(if_not_exist->paramOutGemo(0), if_exist->paramInGemo(0));
    script->addConnect(dir_isExists->paramOutGemo(0), if_not_exist->paramInGemo(0));
    script->addConnect(set_dir->paramOutGemo(0), dir_isExists->paramInGemo(0));
    script->addConnect(out_path->paramOutGemo(0), dir_isExists->paramInGemo(1));

    script->addConnect(if_exist->subFlowOutGemo(0), dir_makepath->flowInGemo());
    script->addConnect(set_dir->paramOutGemo(0), dir_makepath->paramInGemo(0));
    script->addConnect(out_path->paramOutGemo(0), dir_makepath->paramInGemo(1));

    script->addConnect(if_exist->flowOutGemo(0), file_process->flowInGemo());
    script->addConnect(get_in_file->paramOutGemo(0), image_load->paramInGemo(0));
    script->addConnect(image_load->paramOutGemo(0), file_process->paramInGemo(1));    
    script->addConnect(file_process->flowOutGemo(0), image_save->flowInGemo());
    script->addConnect(file_process->paramOutGemo(0), image_save->paramInGemo(0));
    script->addConnect(get_out_file->paramOutGemo(0), image_save->paramInGemo(1));

}