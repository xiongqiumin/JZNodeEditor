#include "JZCameraNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"
#include "../JZModuleDebug.h"

//JZNodeCameraInit
JZNodeCameraInit::JZNodeCameraInit()
{
    m_name = "CameraInit";
    m_type = Node_CameraInit;

    addFlowIn();
    addFlowOut();
}

JZNodeCameraInit::~JZNodeCameraInit()
{

}

void JZNodeCameraInit::setConfig(const JZCameraManagerConfig &config)
{
    m_config = config;
}

JZCameraManagerConfig JZNodeCameraInit::config()
{
    return m_config;
}

bool JZNodeCameraInit::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    auto env = c->env();
    if (!c->checkVariableType("this.cameraManager", env->nameToType("JZCameraManager*"), error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irRef("this.cameraManager") << irLiteral(JZNodeUtils::toBuffer(m_config));
    c->addCallConvert("JZCameraInit", in, out);
    return true;
}

void JZNodeCameraInit::saveToStream(QDataStream& s) const
{
    JZNode::saveToStream(s);
    s << m_config;
}

void JZNodeCameraInit::loadFromStream(QDataStream& s)
{
    JZNode::loadFromStream(s);
    s >> m_config;
}

//JZCameraNode
JZCameraNode::JZCameraNode()
{
    addFlowIn();
    addFlowOut();

    int in = addParamIn("name", Pin_constValue | Pin_noCompiler);
    setPinTypeString(in);

    setPinValue(in, "camera");
}

JZCameraNode::~JZCameraNode()
{
}

bool JZCameraNode::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    auto env = c->env();
    if (!c->checkVariableType("this.cameraManager", env->nameToType("JZCameraManager*"), error))
        return false;

    int cam_id = c->paramId(m_id, paramIn(0));

    QList<JZNodeIRParam> in, out;
    in << irRef("this.cameraManager") << irLiteral(c->pinLiteral(m_id, paramIn(0)));
    c->addCallConvert(m_function, in, out);
    return true;
}

void JZCameraNode::saveToStream(QDataStream& s) const
{
    JZNode::saveToStream(s);
}

void JZCameraNode::loadFromStream(QDataStream& s)
{
    JZNode::loadFromStream(s);
}

void JZCameraNode::setCamera(QString name)
{
    setParamInValue(0, name);
}

QString JZCameraNode::camera()
{
    return paramInValue(0);
}

//JZNodeCameraStart
JZNodeCameraStart::JZNodeCameraStart()
{
    m_name = "CameraStart";
    m_type = Node_CameraStart;
    m_function = "JZCameraStart";
}

//JZNodeCameraStartOnce
JZNodeCameraStartOnce::JZNodeCameraStartOnce()
{
    m_name = "CameraStartOnce";
    m_type = Node_CameraStartOnce;
    m_function = "JZCameraStartOnce";
}

//JZNodeCameraStop
JZNodeCameraStop::JZNodeCameraStop()
{
    m_name = "CameraStop";
    m_type = Node_CameraStop;
    m_function = "JZCameraStop";
}

//JZNodeCameraSetting
JZNodeCameraSetting::JZNodeCameraSetting()
{
    m_name = "CameraSetting";
    m_type = Node_CameraSetting;
    m_function = "JZCameraSetting";
}

//JZCameraCalibration
JZCameraCalibration::JZCameraCalibration()
{
    m_name = "相机标定";
    m_type = Node_CameraCalibration;
    m_function = "JZCameraCalibration";
}

//JZNodeCameraReadyEvent
JZNodeCameraReadyEvent::JZNodeCameraReadyEvent()
{    
    m_type = Node_CameraFrameReady;
    m_name = "相机数据源";
    m_event = "sigFrameReadyEvent";

    int in = addParamIn("name", Pin_constValue | Pin_noCompiler);
    setPinTypeString(in);
    setPinValue(in, "camera");

    int pin = addParamOut("frame");
    setPinType(pin, { "Mat" });

    m_connectInfo.function = "JZCameraConnect";
    m_connectInfo.irList << irRef("this") << irRef("this.cameraManager") << irId(0) << irLiteral(0);
}

JZNodeCameraReadyEvent::~JZNodeCameraReadyEvent()
{
}

void JZNodeCameraReadyEvent::setCamera(QString name)
{
    setParamInValue(0, name);
}

QString JZNodeCameraReadyEvent::camera()
{
    return paramInValue(0);
}

bool JZNodeCameraReadyEvent::compiler(JZNodeCompiler* c, QString& error)
{    
    if (!c->addFlowInput(m_id, error))
        return false;

    auto env = c->env();
    if (!c->checkVariableType("this.cameraManager", env->nameToType("JZCameraManager*"), error))
        return false;
    
    m_connectInfo.irList[2] = irLiteral(c->pinLiteral(m_id, paramIn(0)));
    m_connectInfo.irList[3].m_literal = QVariant::fromValue(JZFunctionPointer(function().fullName()));
    if(!compilerSignal(c, error))
        return false;

    int out_id = c->paramId(m_id, paramOut(0));
    JZModuleDebug(c,m_id, irId(out_id),JZNodeIRParam());
    return true;
}

void JZNodeCameraReadyEvent::saveToStream(QDataStream &s) const
{
    JZNodeSignalEvent::saveToStream(s);
}

void JZNodeCameraReadyEvent::loadFromStream(QDataStream &s)
{
    JZNodeSignalEvent::loadFromStream(s);
}