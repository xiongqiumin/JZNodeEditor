#include "JZModuleComm.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "JZCommManager.h"
#include "3rd/JZCommon/jzModbus/JZModbusMaster.h"
#include "3rd/JZCommon/jzModbus/JZModbusSlaver.h"
#include "../JZModuleConfigFactory.h"

//JZModuleComm
JZModuleComm::JZModuleComm()
{    
    auto cfg_inst = JZModuleConfigFactory<JZCommConfig>::instance();
    cfg_inst->regist(Comm_ModbusRtuClient, JZModuleConfigCreator<JZCommModbusClientConfig>);
    cfg_inst->regist(Comm_ModbusTcpClient, JZModuleConfigCreator<JZCommModbusClientConfig>);
    cfg_inst->regist(Comm_ModbusRtuServer, JZModuleConfigCreator<JZCommModbusServerConfig>);
    cfg_inst->regist(Comm_ModbusTcpServer, JZModuleConfigCreator<JZCommModbusServerConfig>);
    cfg_inst->regist(Comm_TcpClient, JZModuleConfigCreator<JZCommTcpClientConfig>);
    cfg_inst->regist(Comm_TcpServer, JZModuleConfigCreator<JZCommTcpServerConfig>);
    cfg_inst->regist(Comm_Udp, JZModuleConfigCreator<JZCommUdpConfig>);
    cfg_inst->regist(Comm_SerialPort, JZModuleConfigCreator<JZSerialPortConfig>);
}

JZModuleComm::~JZModuleComm()
{
}

void JZModuleComm::regist(JZScriptEnvironment *env)
{                
    jzbind::ClassBind<JZCommManager> cls_comm_mgr(Type_none, "JZCommManager");
    cls_comm_mgr.regist();

    //func
    auto func_inst = env->functionManager();
    func_inst->registCFunction("JZCommInit", true, jzbind::createFuncion(JZCommInit));
    func_inst->registCFunction("JZCommModbusRead", true, jzbind::createFuncion(JZCommModbusRead));
    func_inst->registCFunction("JZCommModbusWrite", true, jzbind::createFuncion(JZCommModbusWrite));

    func_inst->registCFunction("JZCommTcpRead", true, jzbind::createFuncion(JZCommTcpRead));
    func_inst->registCFunction("JZCommTcpWrite", true, jzbind::createFuncion(JZCommTcpWrite));
    func_inst->registCFunction("JZCommTcpReadText", true, jzbind::createFuncion(JZCommTcpReadText));
    func_inst->registCFunction("JZCommTcpWriteText", true, jzbind::createFuncion(JZCommTcpWriteText));
    func_inst->registCFunction("JZCommUdpRead", true, jzbind::createFuncion(JZCommUdpRead));
    func_inst->registCFunction("JZCommUdpWrite", true, jzbind::createFuncion(JZCommUdpWrite));

    func_inst->registCFunction("JZCommSerialRead", true, jzbind::createFuncion(JZCommSerialRead));
    func_inst->registCFunction("JZCommSerialWrite", true, jzbind::createFuncion(JZCommSerialWrite));
    func_inst->registCFunction("JZCommSerialReadText", true, jzbind::createFuncion(JZCommSerialReadText));
    func_inst->registCFunction("JZCommSerialWriteText", true, jzbind::createFuncion(JZCommSerialWriteText));

    //node

    env->nodeFactory()->registNode(Node_CommInit, createJZNode<JZNodeCommInit>);
    env->nodeFactory()->registNode(Node_ModbusRead,createJZNode<JZNodeModbusRead>);
    env->nodeFactory()->registNode(Node_ModbusWrite,createJZNode<JZNodeModbusWrite>);
    env->nodeFactory()->registNode(Node_TcpClientRead, createJZNode<JZNodeTcpClientRead>);
    env->nodeFactory()->registNode(Node_TcpClientWrite, createJZNode<JZNodeTcpClientWrite>);
    env->nodeFactory()->registNode(Node_TcpClientReadText, createJZNode<JZNodeTcpClientReadText>);
    env->nodeFactory()->registNode(Node_TcpClientWriteText, createJZNode<JZNodeSerialWriteText>);
    env->nodeFactory()->registNode(Node_UdpRead, createJZNode<JZNodeUdpRead>);
    env->nodeFactory()->registNode(Node_UdpWrite, createJZNode<JZNodeUdpWrite>);
    env->nodeFactory()->registNode(Node_SerialRead, createJZNode<JZNodeSerialRead>);
    env->nodeFactory()->registNode(Node_SerialWrite, createJZNode<JZNodeSerialWrite>);
    env->nodeFactory()->registNode(Node_SerialReadText, createJZNode<JZNodeSerialReadText>);
    env->nodeFactory()->registNode(Node_SerialWriteText, createJZNode<JZNodeSerialWriteText>);
}

void JZModuleComm::unregist(JZScriptEnvironment *env)
{

}