#include "JZVisionNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"
#include "../JZModuleDebug.h"

//JZNodeVisionImageThreshold
JZNodeVisionImageThreshold::JZNodeVisionImageThreshold()
{
    m_type = Node_VisionImageThreshold;
    m_name = "图像二值化";

    addFlowIn();
    addFlowOut();
    
    int in1 = addParamIn("mat");
    setPinType(in1, { "Mat" });

    int out = addParamOut("out");
    setPinType(out, { "Mat" });
}

bool JZNodeVisionImageThreshold::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionImageCrop
JZNodeVisionImageCrop::JZNodeVisionImageCrop()
{
    m_type = Node_VisionImageCrop;
    m_name = "图像裁减";

    addFlowIn();
    addFlowOut();
    
    int in1 = addParamIn("mat");
    int in2 = addParamIn("roi");
    setPinType(in1, { "Mat" });
    setPinType(in2, { "Rect" });

    int out = addParamOut("out");
    setPinType(out, { "Mat" });
}

bool JZNodeVisionImageCrop::compiler(JZNodeCompiler *c, QString &error)
{    
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionImageCrop",in,out);

    return true;
}

//JZNodeVisionImageFlip 
JZNodeVisionImageFlip::JZNodeVisionImageFlip()
{
    m_type = Node_VisionImageFlip;
    m_name = "图像翻转";

    int in1 = addParamIn("mat");
    int in2 = addParamIn("horizontal");
    int in3 = addParamIn("vertical");
    setPinType(in1, { "Mat" });
    setPinTypeBool(in2);
    setPinTypeBool(in3);

    int out = addParamOut("out");
    setPinType(out, { "Mat" });

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionImageFlip::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in,out;
    in << irId(c->paramId(m_id,paramIn(0)));
    in << irId(c->paramId(m_id,paramIn(1)));
    in << irId(c->paramId(m_id,paramIn(2)));
    out << irId(c->paramId(m_id,paramOut(0)));
    c->addCall("JZVisionImageFlip",in,out);

    JZModuleDebug(c, m_id, out[0], JZNodeIRParam());
    return true;
}

//JZNodeVisionImageConvert
JZNodeVisionImageConvert::JZNodeVisionImageConvert()
{
    m_type = Node_VisionImageConvert;
    m_name = "图像转换";

    addFlowIn();
    addFlowOut();

    int in1 = addParamIn("mat");
    setPinType(in1, { "Mat" });

    int out = addParamOut("out");
    setPinType(out, { "Mat" });
}

bool JZNodeVisionImageConvert::compiler(JZNodeCompiler *c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionImageFilter
JZNodeVisionImageFilter::JZNodeVisionImageFilter()
{
    m_type = Node_VisionImageFilter;
    m_name = "图像滤波";

    int in1 = addParamIn("mat");
    setPinType(in1, { "Mat" });

    int out = addParamOut("out");
    setPinType(out, { "Mat" });

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionImageFilter::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionImageMorphology
JZNodeVisionImageMorphology::JZNodeVisionImageMorphology()
{
    m_type = Node_VisionImageMorphology;
    m_name = "图像形态学";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionImageMorphology::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionImageRotate
JZNodeVisionImageRotate::JZNodeVisionImageRotate()
{
    m_type = Node_VisionImageRotate;
    m_name = "图像旋转";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionImageRotate::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionImageSplice
JZNodeVisionImageSplice::JZNodeVisionImageSplice()
{
    m_type = Node_VisionImageSplice;
    m_name = "图像拼接";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionImageSplice::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionPerspectiveTransform
JZNodeVisionPerspectiveTransform::JZNodeVisionPerspectiveTransform()
{
    m_type = Node_VisionPerspectiveTransform;
    m_name = "图像变换";

    int in1 = addParamIn("mat");
    int in2 = addParamIn("from");
    int in3 = addParamIn("to");
    setPinType(in1, { "Mat" });
    setPinType(in2, { "Rect" });
    setPinType(in3, { "Rect" });

    int out = addParamOut("out");
    setPinType(out, { "Mat" });

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionPerspectiveTransform::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    in << irId(c->paramId(m_id, paramIn(2)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionPerspective", in, out);

    return true;
}

//JZNodeVisionSkeleton
JZNodeVisionSkeleton::JZNodeVisionSkeleton()
{
    m_type = Node_VisionSkeleton;
    m_name = "图像细化";

    addFlowIn();
    addFlowOut();

    int in1 = addParamIn("mat");
    int in2 = addParamIn("size");
    setPinType(in1, { "Mat" });
    setPinTypeInt(in2);

    int out = addParamOut("out");
    setPinType(out, { "Mat" });
}

bool JZNodeVisionSkeleton::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionSkeleton", in, out);

    return true;
}

//JZNodeVisionBlobDetector
JZNodeVisionBlobDetector::JZNodeVisionBlobDetector()
{
    m_type = Node_VisionBlobDetector;
    m_name = "斑点检测";

    addFlowIn();
    addFlowOut();

    int in1 = addParamIn("mat");
}

bool JZNodeVisionBlobDetector::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionBlobDetector", in, out);

    return true;
}

//JZNodeVisionBrightnessDetector
JZNodeVisionBrightnessDetector::JZNodeVisionBrightnessDetector()
{
    m_type = Node_VisionBrightnessDetector;
    m_name = "亮度检测";

    addFlowIn();
    addFlowOut();

    int in1 = addParamIn("image");
    setPinType(in1, { "Mat" });

    int out1 = addParamOut("cast");
    setPinType(out1, { "double" });

    int out2 = addParamOut("da");
    setPinType(out2, { "double" });
}

bool JZNodeVisionBrightnessDetector::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));

    int ret_id = c->addAllocStack("BrightnessDetectorResult");
    out << irId(ret_id);
    c->addCall("JZVisionBrightnessDetector", in, out);

    int ret1 = c->paramId(m_id, paramOut(0));
    int ret2 = c->paramId(m_id, paramOut(1));

    c->addSetVariable(irId(ret1), irIdRef(ret_id, "cast"));
    c->addSetVariable(irId(ret2), irIdRef(ret_id, "da"));

    return true;
}

//JZNodeVisionColorIdentify
JZNodeVisionColorIdentify::JZNodeVisionColorIdentify()
{
    m_type = Node_VisionColorIdentify;
    m_name = "颜色检测";

    addFlowIn();
    addFlowOut();

    int in1 = addParamIn("image");
    setPinType(in1, { "Mat" });

    int in2 = addParamIn("image");
    setPinType(in2, { "Mat" });
}

bool JZNodeVisionColorIdentify::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionColorIdentify", in, out);

    return true;
}

//JZNodeVisionBarCode
JZNodeVisionBarCode::JZNodeVisionBarCode()
{
    m_type = Node_VisionBarCode;
    m_name = "一维码";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionBarCode::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionQrCode 
JZNodeVisionQrCode::JZNodeVisionQrCode()
{
    m_type = Node_VisionQrCode;
    m_name = "二维码";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionQrCode::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionOCR
JZNodeVisionOCR::JZNodeVisionOCR()
{
    m_type = Node_VisionOCR;
    m_name = "OCR";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionOCR::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionShapeMatch
JZNodeVisionShapeMatch::JZNodeVisionShapeMatch()
{
    m_type = Node_VisionShapeMatch;
    m_name = "图像匹配";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionShapeMatch::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionShapeMatch", in, out);

    return true;
}

//JZNodeVisionTemplateMatch
JZNodeVisionTemplateMatch::JZNodeVisionTemplateMatch()
{
    m_type = Node_VisionTemplateMatch;
    m_name = "灰度匹配";

    addFlowIn();
    addFlowOut();

    int in = addParamIn("image");
    setPinType(in, { "Mat" });

    int out = addParamOut("out");
    setPinType(out, { "QRect" });
}

JZNodeVisionTemplateMatch::~JZNodeVisionTemplateMatch()
{
}

void JZNodeVisionTemplateMatch::setConfig(const JZTemplateConfig& config)
{
    m_config = config;
}

JZTemplateConfig JZNodeVisionTemplateMatch::config()
{
    return m_config;
}

bool JZNodeVisionTemplateMatch::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QString obj_name = uniqueName();

    ClassInitInfo info;
    info.function = "JZTemplateMatchInit";
    info.irList << irThis() << irLiteral(obj_name) << irLiteral(JZNodeUtils::toBuffer(m_config));
    c->addClassInitFunction(info);

    int obj_id;
    c->addGet(obj_name, "JZTemplateMatch", obj_id);
    
    QList<JZNodeIRParam> in, out;
    in << irId(obj_id) << irId(c->paramId(m_id,paramIn(0)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZTemplateMatch::match", in, out);

    return true;
}

void JZNodeVisionTemplateMatch::saveToStream(QDataStream& s) const
{
    JZNode::saveToStream(s);
    s << m_config;
}

void JZNodeVisionTemplateMatch::loadFromStream(QDataStream& s)
{
    JZNode::loadFromStream(s);
    s >> m_config;
}

//JZNodeVisionFindCircle
JZNodeVisionFindCircle::JZNodeVisionFindCircle()
{
    m_type = Node_VisionFindCircle;
    m_name = "寻找圆";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionFindCircle::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionFindCircle", in, out);

    return true;
}

//JZNodeVisionFindLine
JZNodeVisionFindLine::JZNodeVisionFindLine()
{
    m_type = Node_VisionFindLine;
    m_name = "寻找直线";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionFindLine::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionFindLine", in, out);

    return true;
}