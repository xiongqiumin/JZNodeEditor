#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleModel.h"
#include "JZNodeBind.h"
#include "JZScriptEnvironment.h"
#include "JZYolo.h"
#include "JZYoloView.h"
#include "JZModelNode.h"
#include "JZNodeFactory.h"
#include "JZContainer.h"

using namespace cv;

//JZModuleModel
JZModuleModel::JZModuleModel()
{        
    m_name = "model";
}

JZModuleModel::~JZModuleModel()
{
}

void JZModuleModel::regist(JZScriptEnvironment *env)
{
    qRegisterMetaType<cv::Mat>("cv::Mat");

    auto func_inst = env->functionManager();
    int cls_id = Module_ModelType;

    jzbind::ClassBind<JZModel> cls_model(cls_id++, "JZModel");
    cls_model.def("loadNet", true, &JZModel::loadNet);
    cls_model.regist();

    jzbind::ClassBind<JZModelManager> cls_model_manger(cls_id++, "JZModelManager");
    cls_model_manger.regist();

    jzbind::ClassBind<JZYoloResult> cls_yolo_ret(cls_id++, "JZYoloResult");
    registList<JZYoloResult>(env, cls_id++);

    cls_yolo_ret.def("toGraphics", true, &JZYoloResult::toGraphics);
    cls_yolo_ret.regist();

    jzbind::ClassBind<JZYolo> cls_yolo(cls_id++, "JZYolo", "JZModel");
    cls_yolo.def("forward", true, &JZYolo::forward);
    cls_yolo.regist();

    int model_ptr_id = JZNodeType::pointerType(cls_model.id());
    int yolo_ptr_id = JZNodeType::pointerType(cls_yolo.id());
    env->registConvertExplicitly(model_ptr_id, yolo_ptr_id, JZObjectCastDown<JZYolo,JZModel>);

    jzbind::ClassBind<JZYoloView> cls_yolo_view(cls_id++, "JZYoloView", "QWidget");
    cls_yolo_view.def("setYoloResult", true, &JZYoloView::setYoloResult);
    cls_yolo_view.regist();
    
    func_inst->registCFunction("JZModelInit", true, jzbind::createFuncion(JZModelInit));
    func_inst->registCFunction("JZModelGet", false, jzbind::createFuncion(JZModelGet, CFunction::Reference));

    env->nodeFactory()->registNode(Node_ModelInit, createJZNode<JZNodeModelInit>);
    env->nodeFactory()->registNode(Node_ModelForward, createJZNode<JZNodeModelForward>);    
}

void JZModuleModel::unregist(JZScriptEnvironment *env)
{
}