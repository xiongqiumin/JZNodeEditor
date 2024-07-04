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
    connect(m_engine,&JZNodeEngine::sigNodePropChanged, this, &JZNodeDebugServer::onNodePropChanged);
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
        for (int i = 0; i < m_debugInfo.breakPoints.size(); i++)
        {
            auto it = m_debugInfo.breakPoints.begin();
            while (it != m_debugInfo.breakPoints.end())
            {
                auto &break_list = it.value();
                for(int bk_idx = 0; bk_idx < break_list.size(); bk_idx++)
                    m_engine->addBreakPoint(it.key(), break_list[bk_idx]);
                it++;
            }
        }

        JZNodeProgramInfo info;
        info.appPath = m_engine->program()->applicationFilePath();
        result << netDataPack(info.appPath);
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
    else if (cmd == Cmd_getVariable)
    {
        JZNodeDebugParamInfo info = netDataUnPack<JZNodeDebugParamInfo>(params[0].toByteArray());        
        result << getVariable(info);
    }
    else if (cmd == Cmd_setVariable)
    {
        JZNodeSetDebugParamInfo info = netDataUnPack<JZNodeSetDebugParamInfo>(params[0].toByteArray());
        result << setVariable(info);
    }

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
}

void JZNodeDebugServer::onNodePropChanged(QString file, int id, QString value)
{
    if (m_client == -1)
        return;

    JZNodeValueChanged info;    
    info.file = file;
    info.id = id;    
    info.value = value;

    JZNodeDebugPacket status_pack;
    status_pack.cmd = Cmd_nodePropChanged;
    status_pack.params << netDataPack<JZNodeValueChanged>(info);
    m_server.sendPack(m_client, &status_pack);
}

QVariant *JZNodeDebugServer::getVariableRef(int stack, const JZNodeParamCoor &coor)
{
    QVariant *ref = nullptr;
    if (coor.type == JZNodeParamCoor::Name)
    {
        ref = m_engine->getVariableRef(coor.name, stack);
    }
    else if (coor.type == JZNodeParamCoor::Id)
    {
        if (coor.id < Stack_User)
            ref = m_engine->getVariableRef(coor.id,stack);
        else
            ref = m_engine->getRegRef(coor.id);
    }
    return ref;
}

QVariant JZNodeDebugServer::getVariable(const JZNodeDebugParamInfo &info)
{        
    JZNodeDebugParamInfo result = info;
    auto stack = m_engine->stack();    
    for (int i = 0; i < info.coors.size(); i++)
    {
        JZNodeDebugParamValue param;
        QVariant *ref = getVariableRef(info.stack,info.coors[i]);
        if (ref)
            param = toDebugParam(*ref);
        else
            param.type = Type_none;
        
        result.values.push_back(param);
    }
    return netDataPack(result);
}

QVariant JZNodeDebugServer::setVariable(const JZNodeSetDebugParamInfo &info)
{    
    JZNodeDebugParamInfo result;
    result.coors << info.coor;
    result.stack = info.stack;    
    return getVariable(result);
}

JZNodeDebugParamValue JZNodeDebugServer::toDebugParam(const QVariant &value)
{
    auto func_inst = JZNodeFunctionManager::instance();

    JZNodeDebugParamValue ret;
    if (isJZObject(value))
    {
        auto obj = toJZObject(value);
        if (obj->isNull())
        {
            ret.type = JZNodeType::variantType(value);
            ret.value = "null";
        }
        else if(JZObjectIsList(obj))
        {
            ret.type = obj->type();            
            
            JZList *list = (JZList*)obj->cobj();
            for (int i = 0; i < list->list.size(); i++)
                ret.params[QString::number(i)] = toDebugParam(list->list[i]);
        }
        else if(JZObjectIsMap(obj))
        {
            ret.type = obj->type();
            
            JZMap *map = (JZMap*)obj->cobj();          
                        
            auto it = map->map.begin();
            while (it != map->map.end())
            {
                QString key = JZNodeType::debugString(it.key().v);
                ret.params[key] = toDebugParam(it.value());
                it++;
            }
        }
        else 
        {
            ret.type = obj->type();            
            auto def = obj->meta();
            auto params = def->paramList(false);
            for (int i = 0; i < params.size(); i++)
            {
                QString name = params[i];
                ret.params[name] = toDebugParam(obj->param(name));
            }
            ret.value = "0x" + QString::number((quint64)obj,16);
        }                  
    }
    else
    {
        ret.type = JZNodeType::variantType(value);
        ret.value = JZNodeType::debugString(value);
    }

    return  ret;
}