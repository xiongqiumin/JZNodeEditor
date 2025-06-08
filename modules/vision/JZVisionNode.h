#ifndef JZ_VISION_NODE_H_
#define JZ_VISION_NODE_H_

#include "JZNode.h"
#include "../JZModuleDefine.h"
#include "JZVision.h"

enum VisionNode
{
    Node_Vision = Module_VisionNode,

    Node_VisionImageThreshold,
    Node_VisionImageCrop, 
    Node_VisionImageFlip,
    Node_VisionImageConvert,
    Node_VisionImageFilter,
    Node_VisionImageMorphology, 
    Node_VisionImageRotate, 
    Node_VisionImageSplice,
    Node_VisionPerspectiveTransform, 
    Node_VisionSkeleton, 

    Node_VisionBlobDetector,
    Node_VisionBrightnessDetector, 
    Node_VisionColorIdentify,

    Node_VisionBarCode,
    Node_VisionQrCode,
    Node_VisionOCR,

    Node_VisionShapeMatch, 
    Node_VisionTemplateMatch,

    Node_VisionFindCircle,
    Node_VisionFindLine, 
};

class JZNodeVisionImageThreshold: public JZNode
{
public:
    JZNodeVisionImageThreshold();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionImageCrop : public JZNode
{
public:
    JZNodeVisionImageCrop();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionImageFlip : public JZNode
{
public:
    JZNodeVisionImageFlip();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionImageConvert : public JZNode
{
public:
    JZNodeVisionImageConvert();

    virtual bool compiler(JZNodeCompiler*, QString& error) override;
};


class JZNodeVisionImageFilter : public JZNode
{
public:
    JZNodeVisionImageFilter();

    virtual bool compiler(JZNodeCompiler*, QString& error) override;
};

class JZNodeVisionImageMorphology : public JZNode
{
public:
    JZNodeVisionImageMorphology();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionImageRotate : public JZNode
{
public:
    JZNodeVisionImageRotate();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionImageSplice : public JZNode
{
public:
    JZNodeVisionImageSplice();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionPerspectiveTransform : public JZNode
{
public:
    JZNodeVisionPerspectiveTransform();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionSkeleton : public JZNode
{
public:
    JZNodeVisionSkeleton();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};


class JZNodeVisionBlobDetector : public JZNode
{
public:
    JZNodeVisionBlobDetector();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionBrightnessDetector : public JZNode
{
public:
    JZNodeVisionBrightnessDetector();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionColorIdentify : public JZNode
{
public:
    JZNodeVisionColorIdentify();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionBarCode : public JZNode
{
public:
    JZNodeVisionBarCode();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionQrCode : public JZNode
{
public:
    JZNodeVisionQrCode();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionOCR : public JZNode
{
public:
    JZNodeVisionOCR();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionShapeMatch : public JZNode
{
public:
    JZNodeVisionShapeMatch();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionTemplateMatch : public JZNode
{
public:
    JZNodeVisionTemplateMatch();
    ~JZNodeVisionTemplateMatch();

    void setConfig(const JZTemplateConfig &config);
    JZTemplateConfig config();

    bool compiler(JZNodeCompiler* c, QString& error);
    void saveToStream(QDataStream& s) const;
    void loadFromStream(QDataStream& s);

protected:
    JZTemplateConfig m_config;
};

class JZNodeVisionFindCircle : public JZNode
{
public:
    JZNodeVisionFindCircle();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

class JZNodeVisionFindLine : public JZNode
{
public:
    JZNodeVisionFindLine();

    virtual bool compiler(JZNodeCompiler *, QString &error) override;
};

#endif // ! JZ_VISION_NODE_H_
