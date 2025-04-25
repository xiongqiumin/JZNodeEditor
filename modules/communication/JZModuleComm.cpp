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
    jzbind::ClassBind<JZCommManager> cls_ptf(Type_none, "JZCommManager");
    cls_ptf.regist();

    auto func_inst = env->functionManager();
    func_inst->registCFunction("JZCommInit", true, jzbind::createFuncion(JZCommInit));

    jzbind::ClassBind<JZModbusParam> cls_modbus_param("JZModbusParam");
    cls_modbus_param.defProperty("name", JZBIND_PROPERTY_IMPL(JZModbusParam,name));    
    cls_modbus_param.defProperty("value", JZBIND_PROPERTY_IMPL(JZModbusParam,value));
    cls_modbus_param.regist();

    jzbind::ClassBind<JZModbusMaster> cls_modbus_master("JZModbusMaster","QObject");
    cls_modbus_master.def("setSlave", true, &JZModbusMaster::setSlave);
    cls_modbus_master.def("isOpen", false, &JZModbusMaster::isOpen);
    cls_modbus_master.def("isBusy", false, &JZModbusMaster::isBusy); 
    cls_modbus_master.def("open", true, &JZModbusMaster::open);
    cls_modbus_master.def("close", true, &JZModbusMaster::close);
    cls_modbus_master.def("param", false, &JZModbusMaster::param, true);
    cls_modbus_master.def("writeParam", true, &JZModbusMaster::writeParam);
    cls_modbus_master.def("readParam", true, &JZModbusMaster::readParam);
    cls_modbus_master.def("writeRemoteParam", true, &JZModbusMaster::writeRemoteParam);
    cls_modbus_master.def("readRemoteParam", true, &JZModbusMaster::readRemoteParam);
    cls_modbus_master.def("writeRemoteParamAsync", true, &JZModbusMaster::writeRemoteParamAsync);
    cls_modbus_master.def("readRemoteParamAsync", true, &JZModbusMaster::readRemoteParamAsync);
    cls_modbus_master.defSingle("sigParamReceived", &JZModbusMaster::sigParamReceived);
    cls_modbus_master.defSingle("sigParamChanged", &JZModbusMaster::sigParamChanged);
    cls_modbus_master.regist();

    jzbind::ClassBind<JZModbusSlaver> cls_modbus_slaver("JZModbusSlaver", "QObject");    
    cls_modbus_slaver.def("setSlave", true, &JZModbusSlaver::setSlave);
    cls_modbus_slaver.def("startServer", true, &JZModbusSlaver::startServer);
    cls_modbus_slaver.def("stopServer", true, &JZModbusSlaver::stopServer);
    cls_modbus_slaver.def("writeParam", true, &JZModbusSlaver::writeParam);
    cls_modbus_slaver.def("readParam", true, &JZModbusSlaver::readParam);
    cls_modbus_slaver.defSingle("sigParamChanged", &JZModbusSlaver::sigParamChanged);
    cls_modbus_slaver.regist();

    func_inst->registCFunction("JZNodeModbusWatchEventInit", true, jzbind::createFuncion(JZNodeModbusWatchEventInit));

    env->factoryManager()->registNode(Node_modbusWatch, createJZNode<JZNodeModbusWatchEvent>);

    env->factoryManager()->registNode(Node_ModbusRead,createJZNode<JZNodeModbusRead>);
    env->factoryManager()->registNode(Node_ModbusWrite,createJZNode<JZNodeModbusWrite>);
    env->factoryManager()->registNode(Node_TcpClientRead, createJZNode<JZNodeTcpClientRead>);
    env->factoryManager()->registNode(Node_TcpClientWrite, createJZNode<JZNodeTcpClientWrite>);
    env->factoryManager()->registNode(Node_UdpRead, createJZNode<JZNodeUdpRead>);
    env->factoryManager()->registNode(Node_UdpWrite, createJZNode<JZNodeUdpWrite>);
    env->factoryManager()->registNode(Node_SerialRead, createJZNode<JZNodeSerialRead>);
    env->factoryManager()->registNode(Node_SerialWrite, createJZNode<JZNodeSerialWrite>);
}

void JZModuleComm::unregist(JZScriptEnvironment *env)
{

}