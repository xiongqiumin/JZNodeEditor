#include "JZVisionNode.h"
#include "JZNodeCompiler.h"

//JZNodeVisionCropImage
JZNodeVisionCropImage::JZNodeVisionCropImage()
{
    m_type = Node_VisionCropImage;
    m_name = "²Ã¼ôÍ¼Ïñ";    

    addFlowIn();
    addFlowOut();
    
    int in1 = addParamIn("mat");
    int in2 = addParamIn("roi");
    setPinType(in1, { "Mat" });
    setPinType(in2, { "Rect" });

    int out = addParamOut("out");
    setPinType(out, { "Mat" });
}

bool JZNodeVisionCropImage::compiler(JZNodeCompiler *c, QString &error)
{    
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionCropImage",in,out);

    return true;
}

//JZNodeVisionImageFlip 
JZNodeVisionImageFlip::JZNodeVisionImageFlip()
{
    m_type = Node_VisionImageFlip;
    m_name = "Í¼Ïñ·­×ª";

    int in1 = addParamIn("mat");
    int in2 = addParamIn("horizontal");
    int in3 = addParamIn("vertical");
    setPinType(in1, { "Mat" });
    setPinTypeInt(in2);
    setPinTypeInt(in3);

    int out = addParamOut("out");
    setPinType(out, { "Mat" });

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionImageFlip::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionImageConvert
JZNodeVisionImageConvert::JZNodeVisionImageConvert()
{
    m_type = Node_VisionImageFlip;
    m_name = "Í¼Ïñ×ª»»";

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
    m_type = Node_VisionImageFlip;
    m_name = "Í¼ÏñÂË²¨";

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
    m_name = "Í¼ÏñÐÎÌ¬Ñ§";

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
    m_name = "Í¼ÏñÐý×ª";

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
    m_name = "Í¼ÏñÆ´½Ó";

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
    m_name = "Í¸ÊÓ±ä»»";

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
    m_name = "Í¼ÏñÏ¸»¯";

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
    m_name = "°ßµã¼ì²â";

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
    m_name = "ÁÁ¶È¼ì²â";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionBrightnessDetector::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionBrightnessDetector", in, out);

    return true;
}

//JZNodeVisionColorIdentify
JZNodeVisionColorIdentify::JZNodeVisionColorIdentify()
{
    m_type = Node_VisionColorIdentify;
    m_name = "ÑÕÉ«Ê¶±ð";

    addFlowIn();
    addFlowOut();
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

//JZNodeVisionShapeMatch
JZNodeVisionShapeMatch::JZNodeVisionShapeMatch()
{
    m_type = Node_VisionShapeMatch;
    m_name = "ÐÎ×´Æ¥Åä";

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
    m_name = "»Ò¶ÈÆ¥Åä";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionTemplateMatch::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("JZVisionTemplateMatch", in, out);

    return true;
}

//JZNodeVisionFindCircle
JZNodeVisionFindCircle::JZNodeVisionFindCircle()
{
    m_type = Node_VisionFindCircle;
    m_name = "Ñ°ÕÒÔ²";

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
    m_name = "Ñ°ÕÒÖ±Ïß";

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