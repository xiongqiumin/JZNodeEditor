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
    in << irRef("this.cameraManager") << irId(id);
    c->addCallConvert("JZCameraInit", in, out);
    return true;
}

//JZCameraNode
JZCameraNode::JZCameraNode()
{
    addFlowIn();
    addFlowOut();

    m_camera = "camera";
}

JZCameraNode::~JZCameraNode()
{
}

bool JZCameraNode::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    auto env = c->env();
    if (!c->checkVariableType("this.cameraManager", env->nameToType("JZCameraManager"), error))
        return false;

    QList<JZNodeIRParam> in, out;
    in << irRef("this.cameraManager") << irLiteral(m_camera);
    c->addCallConvert(m_function, in, out);
    return true;
}

void JZCameraNode::saveToStream(QDataStream& s) const
{
    JZNode::saveToStream(s);
    s << m_camera;
}

void JZCameraNode::loadFromStream(QDataStream& s)
{
    JZNode::loadFromStream(s);
    s >> m_camera;
}

void JZCameraNode::setCamera(QString name)
{
    m_camera = name;
}

QString JZCameraNode::camera()
{
    return m_camera;
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

//JZNodeCameraReadyEvent
JZNodeCameraReadyEvent::JZNodeCameraReadyEvent()
{    
    m_type = Node_CameraFrameReady;
    m_name = "sigFrameReadyEvent";
    m_camera = "camera";

    m_connectInfo.connectFunction = "JZCameraConnect";
    m_connectInfo.irList << irRef("this") << irRef("this.cameraManager") << irLiteral("camera") << irLiteral(0);
}

JZNodeCameraReadyEvent::~JZNodeCameraReadyEvent()
{
}

void JZNodeCameraReadyEvent::setCamera(QString name)
{
    m_camera = name;
}

QString JZNodeCameraReadyEvent::camera()
{
    return m_camera;
}

bool JZNodeCameraReadyEvent::compiler(JZNodeCompiler* c, QString& error)
{    
    if (!c->addFlowInput(m_id, error))
        return false;

    auto env = c->env();
    if (!c->checkVariableType("this.cameraManager", env->nameToType("JZCameraManager"), error))
        return false;

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

//JZNodeCameraVistor
JZNodeCameraVistor::JZNodeCameraVistor()
{

}

void JZNodeCameraVistor::visitorSelf(const JZNode* node)
{
    if (node->type() == Node_CameraFrameReady)
    {
        JZNodeCameraReadyEvent* cam_event = (JZNodeCameraReadyEvent*)node;
        QString camera = cam_event->camera();

        auto init_cam = [](JZNodeObject *object) {
            
        };
        m_depend->initFuncList.push_back(init_cam);
    }
}