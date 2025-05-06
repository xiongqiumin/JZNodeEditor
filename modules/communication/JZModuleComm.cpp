#include "JZModuleComm.h"
#include "JZCommNode.h"
#include "JZScriptEnvironment.h"
#include "JZNodeBind.h"
#include "modbus/JZModuleModbus.h"

//JZModuleComm
JZModuleComm::JZModuleComm()
{    
}

JZModuleComm::~JZModuleComm()
{
}

void JZModuleComm::regist(JZScriptEnvironment *env)
{        
    jzbind::ClassBind<JZModbusParam> cls_modbus_param("JZModbusParam");
    cls_modbus_param.defProperty("name", JZBIND_PROPERTY_IMPL(JZModbusParam,name));    
    cls_modbus_param.defProperty("value", JZBIND_PROPERTY_IMPL(JZModbusParam,value));
    cls_modbus_param.regist();

    jzbind::ClassBind<JZModbusClient> cls_modbus_client("JZModbusClient", "QObject");
    cls_modbus_client.regist();
    
    jzbind::ClassBind<JZModbusServer> cls_modbus_server("JZModbusServer", "QObject");
    cls_modbus_server.regist();

    jzbind::ClassBind<JZModbusMaster> cls_modbus_master("JZModbusMaster","QObject");
    cls_modbus_master.def("setSlave", true, &JZModbusMaster::setSlave);
    cls_modbus_master.def("isOpen", false, &JZModbusMaster::isOpen);
    cls_modbus_master.def("isBusy", false, &JZModbusMaster::isBusy); 
    cls_modbus_master.def("open", true, &JZModbusMaster::open);
    cls_modbus_master.def("close", true, &JZModbusMaster::close);
    cls_modbus_master.def("param", false, &JZModbusMaster::param, CFunction::Reference);
    cls_modbus_master.defSingle("sigParamReceived", &JZModbusMaster::sigParamReceived);
    cls_modbus_master.defSingle("sigParamChanged", &JZModbusMaster::sigParamChanged);
    cls_modbus_master.regist();

    jzbind::ClassBind<JZModbusSlaver> cls_modbus_slaver("JZModbusSlaver", "QObject");    
    cls_modbus_slaver.def("setSlave", true, &JZModbusSlaver::setSlave);
    cls_modbus_slaver.def("startServer", true, &JZModbusSlaver::startServer);
    cls_modbus_slaver.def("stopServer", true, &JZModbusSlaver::stopServer);
    cls_modbus_slaver.defSingle("sigParamChanged", &JZModbusSlaver::sigParamChanged);
    cls_modbus_slaver.regist();

    jzbind::ClassBind<JZCommManager> cls_comm_mgr(Type_none, "JZCommManager");
    //modbusClient
    cls_comm_mgr.def("modbusClient", false, &JZCommManager::modbusClient, CFunction::Reference);
    cls_comm_mgr.regist();

    //func
    auto func_inst = env->functionManager();
    func_inst->registCFunction("JZCommInit", true, jzbind::createFuncion(JZCommInit));
    func_inst->registCFunction("JZNodeModbusWatchEventInit", true, jzbind::createFuncion(JZNodeModbusWatchEventInit));
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
    env->nodeFactory()->registNode(Node_modbusWatch, createJZNode<JZNodeModbusWatchEvent>);

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