#include <QApplication>
#include <QElapsedTimer>
#include "JZNodeDebugServer.h"
#include "JZNodeDebugPacket.h"
#include "JZNodeEngine.h"
#include "JZNodeVM.h"
#include "JZContainer.h"

JZNodeDebugServer::JZNodeDebugServer()
{
    m_client = -1;
    m_engine = nullptr;       
    m_init = false;
    m_preThread = nullptr;

    m_server = new JZNetServer(this);
    connect(m_server,&JZNetServer::sigNewConnect,this,&JZNodeDebugServer::onNewConnect);
	connect(m_server,&JZNetServer::sigDisConnect,this,&JZNodeDebugServer::onDisConnect);
	connect(m_server,&JZNetServer::sigNetPackRecv,this,&JZNodeDebugServer::onNetPackRecv); 
}

JZNodeDebugServer::~JZNodeDebugServer()
{    
    stopServer();
}

void JZNodeDebugServer::run()
{
    exec();
    m_server->stopServer();
    moveToThread(m_preThread);
    m_preThread = nullptr;
}

bool JZNodeDebugServer::startServer(int port)
{
    if(!m_server->startServer(port))
        return false;

    m_preThread = QThread::currentThread();
    moveToThread(this);
    QThread::start();
    return true;
}

void JZNodeDebugServer::stopServer()
{
    if(!isRunning())
        return;
    
    quit();
    wait();    
} 

void JZNodeDebugServer::log(QString log)
{
    if(m_client == -1)
        return;

    JZNodeDebugPacket result_pack;
    result_pack.cmd = Cmd_log;
    //result_pack.params << log;
    m_server->sendPack(m_client,&result_pack);
}

void JZNodeDebugServer::setEngine(JZNodeEngine *eng)
{
    m_engine = eng;
    connect(m_engine,&JZNodeEngine::sigRuntimeError,this,&JZNodeDebugServer::onRuntimeError);
    connect(m_engine,&JZNodeEngine::sigLog, this, &JZNodeDebugServer::onLog);
    connect(m_engine,&JZNodeEngine::sigStatusChanged, this, &JZNodeDebugServer::onStatusChanged);
    connect(m_engine,&JZNodeEngine::sigWatchNotify, this, &JZNodeDebugServer::onWatchNotify);
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
        m_server->closeConnect(netId);

    m_client = netId;
}

void JZNodeDebugServer::onDisConnect(int netId)
{
    if(m_client == netId)
        m_client = -1;    
    
    exit(1);
}

void JZNodeDebugServer::onNetPackRecv(int netId,JZNetPackPtr ptr)
{
    JZNodeDebugPacket *packet = (JZNodeDebugPacket*)(ptr.data());    
    int cmd = packet->cmd;
    QByteArray &params = packet->buffer;
    QByteArray result;
    
    if(cmd == Cmd_init)
    {
        m_debugInfo = netDataUnPack<JZNodeDebugInfo>(params);                                               
        for (int i = 0; i < m_debugInfo.breakPoints.size(); i++)
        {
            auto &pt = m_debugInfo.breakPoints[i];
            m_engine->addBreakPoint(pt);
        }

        JZNodeProgramInfo info;
        info.appPath = m_engine->program()->applicationFilePath();
        result = netDataPack(info.appPath);
        m_init = true;
    }
    else if(cmd == Cmd_addBreakPoint)
    {
        auto pt = netDataUnPack<BreakPoint>(params);
        m_engine->addBreakPoint(pt);
    }
    else if (cmd == Cmd_removeBreakPoint)
    {
        auto pt = netDataUnPack<BreakPoint>(params);
        m_engine->removeBreakPoint(pt.scriptItemPath, pt.nodeId);
    }
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
    else if (cmd == Cmd_runtimeInfo)   
        result = netDataPack(m_engine->runtimeInfo());    
    else if (cmd == Cmd_getVariable)
    {
        JZNodeGetDebugParam info = netDataUnPack<JZNodeGetDebugParam>(params);        
        result = netDataPack(getVariable(info));
    }
    else if (cmd == Cmd_setVariable)
    {
        JZNodeSetDebugParam info = netDataUnPack<JZNodeSetDebugParam>(params);
        result = netDataPack(setVariable(info));
    }

    JZNodeDebugPacket result_pack;
    result_pack.cmd = cmd;
    result_pack.setId(packet->id());
    result_pack.buffer = result;
    m_server->sendPack(netId,&result_pack);
}

void JZNodeDebugServer::onRuntimeError(JZNodeRuntimeError error)
{
    if(m_client == -1)
        return;

    JZNodeDebugPacket result_pack;
    result_pack.cmd = Cmd_runtimeError;
    result_pack.buffer = netDataPack(error);
    m_server->sendPack(m_client,&result_pack);
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
    status_pack.buffer.setNum(status);
    m_server->sendPack(m_client, &status_pack);
}

void JZNodeDebugServer::onWatchNotify()
{
    if (m_client == -1 || m_watch.function.isEmpty())
        return;
        
    int stack_level = -1;    
    for (int i = m_engine->stack()->size(); i >= 0; i--)
    {
        QString function = m_engine->stack()->currentEnv()->function->fullName();
        if (function == m_watch.function)
        {
            stack_level = i;
            break;
        }
    }

    if(stack_level == -1)
        return;

    JZNodeRuntimeWatchResult info;
    info.runtimInfo = m_engine->runtimeInfo();
    
    auto env = m_engine->stack()->env(stack_level);
    for (int i = 0; i < m_watch.watchs.size(); i++)
    {
        auto &w = m_watch.watchs[i];
        auto ref = env->getRef(w.id());
        if (ref)
            info.values[w.id()] = toDebugParam(*ref->ptr);
        else
            info.values[w.id()] = JZNodeDebugParamValue();
    }

    JZNodeDebugPacket status_pack;
    status_pack.cmd = Cmd_watchChanged;
    status_pack.buffer = netDataPack<JZNodeRuntimeWatchResult>(info);
    m_server->sendPack(m_client, &status_pack);
}

JZNodeGetDebugParamResp JZNodeDebugServer::getVariable(const JZNodeGetDebugParam &info)
{        
    JZNodeGetDebugParamResp result;
    result.req = info;
    if (!m_engine->isPauseOrError())
    {
        result.ret = false;
        return result;
    }
    result.ret = true;
    for (int i = 0; i < info.coors.size(); i++)
    {
        if (m_engine->hasParam(info.stack, info.coors[i]))
        {
            auto v = m_engine->getParam(info.stack, info.coors[i]);
            result.values << toDebugParam(v);
        }
        else
        {
            result.values << JZNodeDebugParamValue();
        }
    }    
    return result;
}

JZNodeSetDebugParamResp JZNodeDebugServer::setVariable(const JZNodeSetDebugParam &info)
{    
    JZNodeSetDebugParamResp result;
    result.req = info;
    QVariantPtr *ref = m_engine->getParamRef(info.stack, info.coor);
    if (!ref)
    {
        QVariant value = m_engine->environment()->tryConvertTo(info.value, ref->type);
        if (value.isValid())
        {
            m_engine->dealSet(ref, value);            
            result.ret = true;
        }
        else
        {
            result.ret = false;
        }
    }
    else
    {
        result.ret = false;
    }

    return result;
}

JZNodeDebugParamValue JZNodeDebugServer::toDebugParam(const QVariant &value)
{
    auto func_inst = m_engine->environment()->functionManager();

    JZNodeDebugParamValue ret;
    if (isJZObject(value))
    {
        auto obj = toJZObject(value);
        if (!obj)
        {
            ret.type = JZNodeType::variantType(value);
            ret.value = "null";
        }
        else if(JZObjectIsList(obj))
        {
            ret.type = obj->type();
            
            listForeach(obj, [this, &ret](int key, QVariant value)->bool
            {
                QString str_key = JZNodeType::debugString(key);
                ret.subParamNames << str_key;
                ret.subParamValues << toDebugParam(value);
                return true;
            });

        }
        else if(JZObjectIsMap(obj))
        {
            ret.type = obj->type();

            mapForeach(obj, [this,&ret](QVariant key, QVariant value)->bool
            {
                QString str_key = JZNodeType::debugString(key);
                ret.subParamNames << str_key;
                ret.subParamValues << toDebugParam(value);
                return true;
            });
        }
        else 
        {
            ret.type = obj->type();            
            auto def = obj->meta();
            auto params = def->paramList(false);
            for (int i = 0; i < params.size(); i++)
            {
                QString name = params[i];
                ret.subParamNames << name;
                ret.subParamValues << toDebugParam(obj->param(name));
            }
            ret.value = JZNodeType::debugString(obj);
        }                  
    }
    else
    {
        ret.type = JZNodeType::variantType(value);
        ret.value = JZNodeType::debugString(value);
    }

    return  ret;
}