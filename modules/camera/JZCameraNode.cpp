#include "JZCameraNode.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"

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
    if (!c->checkVariableType("this.cameraManager", env->nameToType("JZCameraManager"), error))
        return false;

    int id = c->allocStack(Type_byteArray);
    c->addSetBuffer(irId(id), JZNodeUtils::toBuffer(m_config));

    QList<JZNodeIRParam> in, out;
    in << irRef("this.commManager") << irId(id);
    c->addCallConvert("JZCameraInit", in, out);
    return true;

    return true;
}

//JZNodeCameraReadyEvent
JZNodeCameraReadyEvent::JZNodeCameraReadyEvent()
{    
    m_type = Node_CameraFrameReady;
    m_name = "sigFrameReadyEvent";

    m_connectInfo.connectFunction = "JZCameraConnect";
    m_connectInfo.irList << irRef("this") << irRef("this.cameraManager") << irLiteral("camera") << irLiteral(0);
}

JZNodeCameraReadyEvent::~JZNodeCameraReadyEvent()
{
}

bool JZNodeCameraReadyEvent::compiler(JZNodeCompiler* c, QString& error)
{    
    m_connectInfo.irList[2].m_literal = m_camera;
    m_connectInfo.irList[3].m_literal = QVariant::fromValue(JZFunctionPointer(function().fullName()));
    return compilerSignal(c, error);
}

void JZNodeCameraReadyEvent::saveToStream(QDataStream &s) const
{
    JZNodeSignalEvent::saveToStream(s);
}

void JZNodeCameraReadyEvent::loadFromStream(QDataStream &s)
{
    JZNodeSignalEvent::loadFromStream(s);
}