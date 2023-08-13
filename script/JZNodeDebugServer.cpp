#include <QApplication>
#include <QElapsedTimer>
#include "JZNodeDebugServer.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeEngine.h"
#include "JZNodeVM.h"

JZNodeDebugServer::JZNodeDebugServer()
{
    m_client = -1;
    m_engine = nullptr;    
    m_vm = nullptr;
    m_init = false;
    this->moveToThread(this);

    connect(&m_server,&JZNetServer::sigNewConnect,this,&JZNodeDebugServer::onNewConnect);
	connect(&m_server,&JZNetServer::sigDisConnect,this,&JZNodeDebugServer::onDisConnect);
	connect(&m_server,&JZNetServer::sigNetPackRecv,this,&JZNodeDebugServer::onNetPackRecv);
    connect(this,&JZNodeDebugServer::sigStop,this,&JZNodeDebugServer::onStop);    
}

JZNodeDebugServer::~JZNodeDebugServer()
{    
    stopServer();
}

bool JZNodeDebugServer::startServer(int port)
{
    if(!m_server.startServer(port))
        return false;

    m_server.moveToThread(this);
    QThread::start();
    return true;
}

void JZNodeDebugServer::stopServer()
{
    if(!isRunning())
        return;
    
    emit sigStop(QThread::currentThread(), QPrivateSignal());
    wait();    
} 

void JZNodeDebugServer::onStop(QThread *stopThread)
{
    m_server.stopServer();
    m_server.moveToThread(stopThread);
    quit();
}

void JZNodeDebugServer::log(QString log)
{
    if(m_client == -1)
        return;

    JZNodeDebugPacket result_pack;
    result_pack.cmd = Cmd_log;
    result_pack.params << log;
    m_server.sendPack(m_client,&result_pack);
}

void JZNodeDebugServer::setEngine(JZNodeEngine *eng)
{
    m_engine = eng;
    connect(m_engine,&JZNodeEngine::sigRuntimeError,this,&JZNodeDebugServer::onRuntimeError);
    connect(m_engine,&JZNodeEngine::sigLog, this, &JZNodeDebugServer::onLog);
    connect(m_engine,&JZNodeEngine::sigStatusChanged, this, &JZNodeDebugServer::onStatusChanged);
}

void JZNodeDebugServer::setVM(JZNodeVM *vm)
{
    m_vm = vm;
}

bool JZNodeDebugServer::waitForAttach(int timeout)
{
    QElapsedTimer e;
    e.start();
    while (e.elapsed() <= timeout)
    {
        if (m_init)
            return true;
        QThread::msleep(50);
    }
    return false;
}

JZNodeDebugInfo JZNodeDebugServer::debugInfo()
{
    return m_debugInfo;
}

void JZNodeDebugServer::onNewConnect(int netId)
{
    if(m_client != -1)
        m_server.closeConnect(netId);

    m_client = netId;
}

void JZNodeDebugServer::onDisConnect(int netId)
{
    if(m_client == netId)
        m_client = -1;    
    if (m_vm)
        m_vm->quit();
}

void JZNodeDebugServer::onNetPackRecv(int netId,JZNetPackPtr ptr)
{
    JZNodeDebugPacket *packet = (JZNodeDebugPacket*)(ptr.data());    
    int cmd = packet->cmd;
    QVariantList &params = packet->params;
    QVariantList result;
    
    if(cmd == Cmd_init)
    {
        m_debugInfo = netDataUnPack<JZNodeDebugInfo>(params[0].toByteArray());                                
        
        JZNodeProgramInfo info = getProgramInfo();                
        result << netDataPack(info);
        m_init = true;
    }
    else if(cmd == Cmd_addBreakPoint)
        m_engine->addBreakPoint(params[0].toString(),params[1].toInt());
    else if(cmd == Cmd_removeBreakPoint)    
        m_engine->removeBreakPoint(params[0].toString(),params[1].toInt());
    else if(cmd == Cmd_clearBreakPoint)
        m_engine->clearBreakPoint();
    else if(cmd == Cmd_pause)
        m_engine->pause();
    else if(cmd == Cmd_resume)    
        m_engine->resume();
    else if(cmd == Cmd_stepIn)
        m_engine->stepIn();
    else if(cmd == Cmd_stop)    
        m_engine->stop();     
    else if(cmd == Cmd_stepOver)
        m_engine->stepOver();
    else if(cmd == Cmd_stepOut)                                       
        m_engine->stepOut();
    else if(cmd == Cmd_runtimeInfo)    
        result << netDataPack(m_engine->runtimeInfo());    
    else if(cmd == Cmd_getVariable)
        result << getVariable(params);
    else if(cmd == Cmd_setVariable)
        setVariable(params);

    JZNodeDebugPacket result_pack;
    result_pack.cmd = cmd;
    result_pack.setSeq(packet->seq());
    result_pack.params = result;
    m_server.sendPack(netId,&result_pack);
}

void JZNodeDebugServer::onRuntimeError(JZNodeRuntimeError error)
{
    if(m_client == -1)
        return;

    JZNodeDebugPacket result_pack;
    result_pack.cmd = Cmd_runtimeError;
    result_pack.params << netDataPack(error);
    m_server.sendPack(m_client,&result_pack);
}

void JZNodeDebugServer::onLog(const QString &text)
{
    log(text);
}

void JZNodeDebugServer::onStatusChanged(int status)
{    
    if (m_client == -1)
        return;

    JZNodeDebugPacket status_pack;
    status_pack.cmd = Cmd_runtimeStatus;
    status_pack.params << status;
    m_server.sendPack(m_client, &status_pack);

    if (status == Status_pause || status == Status_idlePause)
    {
        JZNodeDebugPacket runtime_pack;
        runtime_pack.cmd = Cmd_runtimeInfo;
        runtime_pack.params << netDataPack(m_engine->runtimeInfo());
        m_server.sendPack(m_client, &runtime_pack);
    }
}

QVariant JZNodeDebugServer::getVariable(const QVariantList &list)
{    
    JZNodeDebugParamInfo info = netDataUnPack<JZNodeDebugParamInfo>(list[0].toByteArray());
    auto stack = m_engine->stack();
    auto this_obj = toJZObject(m_engine->getThis());
    for (int i = 0; i < info.coors.size(); i++)
    {
        JZNodeDebugParamValue param;
        QVariant value;
        auto &coor = info.coors[i];
        if (coor.type == JZNodeParamCoor::Local || coor.type == JZNodeParamCoor::NodeId)
        {
            auto env = stack->stackVariable(coor.stack);
            if (coor.type == JZNodeParamCoor::Local)
            {
                if (env->locals.contains(coor.name))
                    value = env->locals[coor.name];
            }
            else
            {
                if (env->stacks.contains(coor.id))
                    value = env->stacks[coor.id];
            }
        }
        else if (coor.type == JZNodeParamCoor::This)
        {
            value = this_obj->param(coor.name);
        }
        else if (coor.type == JZNodeParamCoor::Global)
        {
            if (auto ref = m_engine->getVariableRef(coor.name))
                value = *ref;
        }
        else if (coor.type == JZNodeParamCoor::Reg)
        {
            value = m_engine->getReg(coor.id);
        }                        
                
        param = toDebugParam(value);
        info.values << param;
    }
    return netDataPack(info);
}

void JZNodeDebugServer::setVariable(const QVariantList &list)
{
    JZNodeDebugParamInfo info = netDataUnPack<JZNodeDebugParamInfo>(list[0].toByteArray());
    auto stack = m_engine->stack();
    for (int i = 0; i < info.coors.size(); i++)
    {
        QVariant value;
        auto &coor = info.coors[i];
        if (coor.type == JZNodeParamCoor::Local || coor.type == JZNodeParamCoor::NodeId)
        {
            auto env = stack->stackVariable(coor.stack);
            if (coor.type == JZNodeParamCoor::Local)
            {
                if (env->locals.contains(coor.name))
                    env->locals[coor.name] = value;
            }
            else
            {
                if (env->stacks.contains(coor.id))
                    env->stacks[coor.id] = value;
            }
        }
        else if (coor.type == JZNodeParamCoor::Global)
        {
            if (auto ref = m_engine->getVariableRef(coor.name))
                *ref = value;
        }
        else if (coor.type == JZNodeParamCoor::Reg)
        {
            m_engine->setReg(coor.id,value);
        }        
    }
}

JZNodeDebugParamValue JZNodeDebugServer::toDebugParam(const QVariant &value)
{
    JZNodeDebugParamValue ret;
    if (isJZObject(value))
    {
        auto obj = toJZObject(value);
        if (obj)
        {
            ret.type = obj->type();            
            auto def = obj->meta();
            auto params = def->paramList();
            for (int i = 0; i < params.size(); i++)
            {
                QString name = params[i];
                ret.params[name] = toDebugParam(obj->param(name));
            }
        }
        else
            ret.type = Type_nullptr;
        ret.value = (qint64)obj;
    }
    else
    {
        ret.type = JZNodeType::variantType(value);
        ret.value = value;
    }
    return  ret;
}

JZNodeProgramInfo JZNodeDebugServer::getProgramInfo()
{
    JZNodeProgramInfo info;
    JZNodeProgram *program = m_engine->program();

    auto list = program->scriptList();
    for (int i = 0; i < list.size(); i++)
    {
        auto s = list[i];
        JZNodeScriptInfo script_info;
        script_info.file = s->file;
        script_info.className = s->className;
        script_info.nodeInfo = s->nodeInfo;
        script_info.functionList = s->functionList;
        for (int j = 0; j < s->paramChangeEvents.size(); j++)
        {
            script_info.functionList << s->paramChangeEvents[j].function;
        }
        for (int j = 0; j < s->events.size(); j++)
        {
            script_info.functionList << s->events[j].function;
        }
        script_info.runtimeInfo = s->runtimeInfo;

        info.scripts[s->file] = script_info;
    }
    info.globalFunstions = program->functionDefines();
    info.globalVariables = program->globalVariables();
    info.objectDefines = program->objectDefines();
    return info;
}