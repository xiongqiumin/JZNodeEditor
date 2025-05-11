#include "JZVisionNode.h"
#include "JZNodeCompiler.h"

//JZNodeVisionCropImage
JZNodeVisionCropImage::JZNodeVisionCropImage()
{
    m_type = Node_VisionCropImage;
    m_name = "²Ã¼ôÍ¼Ïñ";    

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionCropImage::compiler(JZNodeCompiler *c, QString &error)
{    
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionImageFlip 
JZNodeVisionImageFlip::JZNodeVisionImageFlip()
{
    m_type = Node_VisionImageFlip;
    m_name = "Í¼Ïñ·­×ª";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionImageFlip::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionImageMorphology
JZNodeVisionImageMorphology::JZNodeVisionImageMorphology()
{
    m_type = Node_VisionImageMorphology;
    m_name = "Ô¤´¦Àí";

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

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionPerspectiveTransform::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionSkeleton
JZNodeVisionSkeleton::JZNodeVisionSkeleton()
{
    m_type = Node_VisionSkeleton;
    m_name = "Í¼ÏñÏ¸»¯";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionSkeleton::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    return true;
}

//JZNodeVisionBlobDetector
JZNodeVisionBlobDetector::JZNodeVisionBlobDetector()
{
    m_type = Node_VisionBlobDetector;
    m_name = "°ßµã¼ì²â";

    addFlowIn();
    addFlowOut();
}

bool JZNodeVisionBlobDetector::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

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

    return true;
}