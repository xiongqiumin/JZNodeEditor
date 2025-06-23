#include <opencv2/opencv.hpp>
#include <QBuffer>
#include "JZModuleVision.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "JZContainer.h"

using namespace cv;

//JZModuleVision
JZModuleVision::JZModuleVision()
{        
    m_name = "vision";
}

JZModuleVision::~JZModuleVision()
{
}

void JZModuleVision::regist(JZScriptEnvironment *env)
{    
    int cls_id = Module_VisionType;

    jzbind::ClassBind<JZTemplateMatch> cls_template_match(cls_id++, "JZTemplateMatch", "QObject");
    cls_template_match.def("match", true, &JZTemplateMatch::match);
    cls_template_match.regist();

    //JZBarCode
    jzbind::ClassBind<JZBarCodeResult> cls_bar_ret(cls_id++, "JZBarCodeResult");
    registList<JZBarCodeResult>(env, cls_id++);

    cls_bar_ret.def("toGraphics", true, &JZBarCodeResult::toGraphics);
    cls_bar_ret.regist();

    jzbind::ClassBind<JZBarCode> cls_bar(cls_id++, "JZBarCode", "QObject");
    cls_bar.def("init", true, &JZBarCode::init);
    cls_bar.def("detect", true, &JZBarCode::detect);
    cls_bar.regist();


    //JZQRCode
    jzbind::ClassBind<JZQRCodeResult> cls_qr_ret(cls_id++, "JZQRCodeResult");
    registList<JZQRCodeResult>(env, cls_id++);

    cls_qr_ret.def("toGraphics", true, &JZQRCodeResult::toGraphics);
    cls_qr_ret.regist();

    jzbind::ClassBind<JZQRCode> cls_qr(cls_id++, "JZQRCode", "QObject");
    cls_qr.def("init", true, &JZQRCode::init);
    cls_qr.def("detect", true, &JZQRCode::detect);
    cls_qr.regist();


    jzbind::ClassBind<BrightnessDetectorResult> cls_birghtness_ret(cls_id++, "BrightnessDetectorResult");
    cls_birghtness_ret.setValueType(true);
    cls_birghtness_ret.defProperty("cast", &BrightnessDetectorResult::cast);
    cls_birghtness_ret.defProperty("da", &BrightnessDetectorResult::da);
    cls_birghtness_ret.regist();

    auto func_inst = env->functionManager();
    func_inst->registCFunction("JZTemplateMatchInit", true, jzbind::createFuncion(JZTemplateMatchInit));
   
    func_inst->registCFunction("JZVisionImageThreshold", true, jzbind::createFuncion(JZVisionImageThreshold));
    func_inst->registCFunction("JZVisionImageCrop", true, jzbind::createFuncion(JZVisionImageCrop));
    func_inst->registCFunction("JZVisionImageFlip", true, jzbind::createFuncion(JZVisionImageFlip));
    func_inst->registCFunction("JZVisionImageMorphology", true, jzbind::createFuncion(JZVisionImageMorphology));
    func_inst->registCFunction("JZVisionPerspectiveTransform", true, jzbind::createFuncion(JZVisionPerspectiveTransform));
    func_inst->registCFunction("JZVisionSkeleton", true, jzbind::createFuncion(JZVisionSkeleton));

    //func_inst->registCFunction("JZVisionBlobDetector", true, jzbind::createFuncion(JZVisionBlobDetector));
    func_inst->registCFunction("JZVisionBrightnessDetector", true, jzbind::createFuncion(JZVisionBrightnessDetector));
    func_inst->registCFunction("JZVisionColorIdentify", true, jzbind::createFuncion(JZVisionColorIdentify));

    func_inst->registCFunction("JZVisionFindCircle", true, jzbind::createFuncion(JZVisionFindCircle));
    func_inst->registCFunction("JZVisionFindLine", true, jzbind::createFuncion(JZVisionFindLine));

    auto obj_inst = env->objectManager();

    //node
    auto node_inst = env->nodeFactory();

    node_inst->registNode(Node_VisionImageCrop, createJZNode<JZNodeVisionImageCrop>);

    node_inst->registNode(Node_VisionImageFlip, createJZNode<JZNodeVisionImageFlip>);
    node_inst->registNode(Node_VisionImageMorphology, createJZNode<JZNodeVisionImageMorphology>);
    node_inst->registNode(Node_VisionImageRotate, createJZNode<JZNodeVisionImageRotate>);
    node_inst->registNode(Node_VisionImageSplice, createJZNode<JZNodeVisionImageSplice>);
    node_inst->registNode(Node_VisionPerspectiveTransform, createJZNode<JZNodeVisionPerspectiveTransform>);
    node_inst->registNode(Node_VisionSkeleton, createJZNode<JZNodeVisionSkeleton>);

    node_inst->registNode(Node_VisionBlobDetector, createJZNode<JZNodeVisionBlobDetector>);
    node_inst->registNode(Node_VisionBrightnessDetector, createJZNode<JZNodeVisionBrightnessDetector>);
    node_inst->registNode(Node_VisionColorIdentify, createJZNode<JZNodeVisionColorIdentify>);

    node_inst->registNode(Node_VisionShapeMatch, createJZNode<JZNodeVisionShapeMatch>);
    node_inst->registNode(Node_VisionTemplateMatch, createJZNode<JZNodeVisionTemplateMatch>);
    
    node_inst->registNode(Node_VisionBarCode, createJZNode<JZNodeVisionBarCode>);
    node_inst->registNode(Node_VisionQrCode, createJZNode<JZNodeVisionQrCode>);
    node_inst->registNode(Node_VisionOCR, createJZNode<JZNodeVisionOCR>);

    node_inst->registNode(Node_VisionFindCircle, createJZNode<JZNodeVisionFindCircle>);
    node_inst->registNode(Node_VisionFindLine, createJZNode<JZNodeVisionFindLine>);
}

void JZModuleVision::unregist(JZScriptEnvironment *env)
{
}