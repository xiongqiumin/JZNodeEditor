#include "JZVisionAppNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeTrace.h"
#include "modules/JZModuleDebug.h"

JZVisionParamLink::JZVisionParamLink()
{
    type = Link_None;
}

bool JZVisionParamLink::isNull()
{
    return type == Link_None;
}

bool JZVisionParamLink::operator==(const JZVisionParamLink &other) const
{
    return this->type == other.type &&  this->gemo == other.gemo && this->path == other.path
        && this->paramType == other.paramType;
}

bool JZVisionParamLink::operator!=(const JZVisionParamLink& other) const
{
    return !(*this == other);
}

QDataStream &operator<<(QDataStream &s, const JZVisionParamLink &param)
{
    s << param.type << param.gemo;
    s << param.path << param.paramType;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZVisionParamLink &param)
{
    s >> param.type >> param.gemo;
    s >> param.path >> param.paramType;
    return s;
}

//JZNodeVisionParam
JZNodeVisionParam::JZNodeVisionParam()
{
    m_type = Node_visionAppParam;
}   

JZNodeVisionParam::~JZNodeVisionParam()
{

}

bool JZNodeVisionParam::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addDataInput(m_id,error))
        return false;

    if(m_link.type == JZVisionParamLink::Link_Node)
        c->addSetVariable(irId(paramOutId(0)), irIdRef(paramInId(0),m_link.path.join(".")));
    else
        c->addSetVariable(irId(paramOutId(0)), irRef(m_link.path.join(".")));

    return true;
}

JZVisionParamLink JZNodeVisionParam::link()
{
    return m_link;
}

void JZNodeVisionParam::setLink(JZVisionParamLink link)
{
    m_link = link;
    clearPin();
    if(m_link.type == JZVisionParamLink::Link_Node)
    {
        auto in = addParamIn("in");
        setPinType(in,{ link.paramType });
    }

    int out = addParamOut("out");
    setPinType(out,{ link.paramType });
}

void JZNodeVisionParam::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
    s << m_link;
}

void JZNodeVisionParam::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
    s >> m_link;
    setLink(m_link);
}

//JZNodeVisionBarCode
JZNodeVisionAppBarCode::JZNodeVisionAppBarCode()
{
    m_type = Node_visionAppBarCode;
    m_name = "一维码";

    addFlowIn();
    addFlowOut();

    int in = addParamIn("image");
    setPinType(in, { "Mat" });

    int result = addParamOut("result");
    setPinType(result, { "QList<JZBarCodeResult>" });
}

bool JZNodeVisionAppBarCode::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    int obj_id = c->addAllocStack("JZBarCode*");
    c->addCall("JZVisionApplication::getBarCode", { irThis() }, { irId(obj_id) });

    int in_id = paramInId(0);
    int out_id = paramOutId(0);
    QList<JZNodeIRParam> in, out;
    in << irId(obj_id);
    in << irId(in_id);
    out << irId(out_id);
    c->addCall("JZBarCode::detect", in, out);

    JZModuleDebug(c, m_id, irId(in_id), irId(out_id));
    return true;
}

//JZNodeVisionQrCode 
JZNodeVisionAppQrCode::JZNodeVisionAppQrCode()
{
    m_type = Node_visionAppQrCode;
    m_name = "二维码";

    addFlowIn();
    addFlowOut();

    int in = addParamIn("image");
    setPinType(in, { "Mat" });

    int result = addParamOut("result");
    setPinType(result, { "QList<JZQRCodeResult>" });
}

bool JZNodeVisionAppQrCode::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;
        
    int obj_id = c->addAllocStack("JZQRCode*");
    c->addCall("JZVisionApplication::getQrCode", { irThis() }, { irId(obj_id) });

    int in_id = paramInId(0);
    int out_id = paramOutId(0);
    QList<JZNodeIRParam> in, out;
    in << irId(obj_id);
    in << irId(in_id);
    out << irId(out_id);
    c->addCall("JZQRCode::detect", in, out);

    JZModuleDebug(c, m_id, irId(in_id), irId(out_id));
    return true;
}

//JZNodeVisionOCR
JZNodeVisionAppOCR::JZNodeVisionAppOCR()
{
    m_type = Node_visionAppOCR;
    m_name = "OCR";

    addFlowIn();
    addFlowOut();

    int in = addParamIn("image");
    setPinType(in, { "Mat" });

    int result = addParamOut("result");
    setPinType(result, { "QList<JZOCRResult>" });
}

bool JZNodeVisionAppOCR::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    JZNodeTraceBuilder trace(c);

    int obj_id = c->addAllocStack("JZPaddleOCR*");
    c->addCall("JZVisionApplication::getOCR", { irThis() }, { irId(obj_id) });

    int in_id = paramInId(0);
    int out_id = paramOutId(0);
    QList<JZNodeIRParam> in,out;
    in << irId(obj_id);
    in << irId(in_id);
    out << irId(out_id);
    c->addCall("JZPaddleOCR::ocr", in, out);

    JZModuleDebug(c, m_id, irId(in_id), irId(out_id));
    return true;
}