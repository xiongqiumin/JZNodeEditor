#ifndef JZ_VISION_APP_NODE_H_
#define JZ_VISION_APP_NODE_H_

#include "JZNode.h"
#include "modules/JZModuleDefine.h"

enum {
    Node_visionAppParam = Module_VisionAppNode,
    Node_visionAppBarCode,
    Node_visionAppQrCode,
    Node_visionAppOCR,
};


class JZVisionParamLink
{
public:
    enum LinkType
    {
        Link_None,
        Link_Member,
        Link_Local,
        Link_Node,
    };

    JZVisionParamLink();
    bool isNull();
    bool operator==(const JZVisionParamLink &other) const;
    bool operator!=(const JZVisionParamLink& other) const;

    LinkType type;
    JZNodeGemo gemo;
    QStringList path;
    QString paramType;  //需要中间变量时会用到
};
QDataStream &operator<<(QDataStream &s, const JZVisionParamLink &param);
QDataStream &operator>>(QDataStream &s, JZVisionParamLink &param);

//JZNodeVisionParam
class JZNodeVisionParam : public JZNode
{
public:
    JZNodeVisionParam();
    virtual ~JZNodeVisionParam();

    void setLink(JZVisionParamLink link);
    JZVisionParamLink link();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error); 
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

protected:
    JZVisionParamLink m_link;
};

//JZNodeVisionAppBarCode
class JZNodeVisionAppBarCode : public JZNode
{
public:
    JZNodeVisionAppBarCode();

    virtual bool compiler(JZNodeCompiler*, QString& error) override;
};

//JZNodeVisionQrCode
class JZNodeVisionAppQrCode : public JZNode
{
public:
    JZNodeVisionAppQrCode();

    virtual bool compiler(JZNodeCompiler*, QString& error) override;
};

//JZNodeVisionOCR
class JZNodeVisionAppOCR : public JZNode
{
public:
    JZNodeVisionAppOCR();

    virtual bool compiler(JZNodeCompiler*, QString& error) override;
};

#endif // !JZ_MODEL_VISION_APP_H_