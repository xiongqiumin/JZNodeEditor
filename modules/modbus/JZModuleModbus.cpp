#include <QSerialPort>
#include <QPushButton>
#include "JZModuleModbus.h"
#include "JZNodeObject.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"
#include "JZNodeFactory.h"
#include "jzmodbus/JZModbusMaster.h"
#include "jzmodbus/JZModbusSlaver.h"
#include "JZModbusConfigDialog.h"
#include "JZModbusSimulator.h"
#include "JZNodePinWidget.h"
#include "JZNodeCompiler.h"
#include "JZNodeUtils.h"
#include "JZNodeEditorManager.h"

JZNodeModbusConfig::JZNodeModbusConfig()
{
    m_type = Node_modbusConfig;
}

JZNodeModbusConfig::~JZNodeModbusConfig()
{

}

QString JZNodeModbusConfig::className()
{
    return "JZ" + m_functionName.mid(4);
}

void JZNodeModbusConfig::initFunction()
{
    addFlowIn();
    addFlowOut();
        
    QString class_type = className();
    int in = addParamIn("modbus", Pin_dispName);
    pin(in)->setDataType({class_type});
    
    addParamIn("", Pin_widget | Pin_noValue);
    setName(m_functionName);
}

JZNodePinWidget *JZNodeModbusConfig::createWidget(int id)
{
    if (id == paramIn(1))
    {
        JZNodePinButtonWidget *w = new JZNodePinButtonWidget();
        auto btn = w->button();
        btn->setText("设置");
        btn->connect(btn, &QPushButton::clicked, [btn,this] {
            QByteArray old = this->toBuffer();

            JZModbusConfigDialog dlg;
            dlg.setConfigMode(true);
            dlg.setConfig(this->m_config);
            if (dlg.exec() != QDialog::Accepted)
                return;

            m_config = dlg.config();
            propertyChangedNotify(old);
        });
        return w;
    }

    return nullptr;
}

bool JZNodeModbusConfig::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;    

    QByteArray buffer;
    QDataStream s(&buffer, QIODevice::WriteOnly);
    s << m_config;

    int obj_id = JZNodeGemo::paramId(m_id, paramIn(0));
    int id = c->allocStack(Type_byteArray);
    c->addSetBuffer(irId(id), buffer);

    QList<JZNodeIRParam> in, out;
    in << irId(obj_id) << irId(id);
    c->addCall(m_functionName,in,out);
    c->addJumpNode(flowOut());

    return true;
}

void JZNodeModbusConfig::saveToStream(QDataStream &s) const
{
    JZNodeFunctionCustom::saveToStream(s);
    s << m_functionName << m_config;
}

void JZNodeModbusConfig::loadFromStream(QDataStream &s)
{
    JZNodeFunctionCustom::loadFromStream(s);
    s >> m_functionName >> m_config;
}


void initModbusMaster(JZModbusMaster *master, const QByteArray &buffer)
{
    JZModbusConfig config;
    QDataStream s(buffer);
    s >> config;
    modbusMasterSetConfig(master, &config);
}

void initModbusSlaver(JZModbusSlaver *slaver,const QByteArray &buffer)
{
    JZModbusConfig config;
    QDataStream s(buffer);
    s >> config;
    modbusSlaverSetConfig(slaver, &config);
}

//JZModuleModbus
JZModuleModbus::JZModuleModbus()
{    
    m_name = "modbus";
    m_functionList << "initModbusMaster" << "initModbusSlaver" << "initPlotConfig";
    m_classList << "QSerialPort::BaudRate" << "QSerialPort::StopBits" << "QSerialPort::Parity" << "QSerialPort::DataBits" <<
        "JZModbusParam" << "JZModbusSlaver" << "JZModbusMaster" << "JZPlotWidget";
}

JZModuleModbus::~JZModuleModbus()
{
}

void JZModuleModbus::regist(JZScriptEnvironment *env)
{
    jzbind::registEnum<QSerialPort::BaudRate>("QSerialPort::BaudRate");
    jzbind::registEnum<QSerialPort::StopBits>("QSerialPort::StopBits");
    jzbind::registEnum<QSerialPort::Parity>("QSerialPort::Parity");
    jzbind::registEnum<QSerialPort::DataBits>("QSerialPort::DataBits");

    jzbind::ClassBind<JZModbusParam> cls_modbus_param("JZModbusParam");
    cls_modbus_param.defProperty("name", JZBIND_PROPERTY_IMPL(JZModbusParam,name));    
    cls_modbus_param.defProperty("value", JZBIND_PROPERTY_IMPL(JZModbusParam,value));
    cls_modbus_param.regist();

    jzbind::ClassBind<JZModbusMaster> cls_modbus_master("JZModbusMaster","QObject");
    cls_modbus_master.def("setSlave", true, &JZModbusMaster::setSlave);
    cls_modbus_master.def("isOpen", false, &JZModbusMaster::isOpen);
    cls_modbus_master.def("isBusy", false, &JZModbusMaster::isBusy);    
    cls_modbus_master.def("initRtu", true, &JZModbusMaster::initRtu);
    cls_modbus_master.def("initTcp", true, &JZModbusMaster::initTcp);
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
    cls_modbus_slaver.def("initRtu", true, &JZModbusSlaver::initRtu);
    cls_modbus_slaver.def("initTcp", true, &JZModbusSlaver::initTcp);
    cls_modbus_slaver.def("startServer", true, &JZModbusSlaver::startServer);
    cls_modbus_slaver.def("stopServer", true, &JZModbusSlaver::stopServer);
    cls_modbus_slaver.def("writeParam", true, &JZModbusSlaver::writeParam);
    cls_modbus_slaver.def("readParam", true, &JZModbusSlaver::readParam);
    cls_modbus_slaver.defSingle("sigParamChanged", &JZModbusSlaver::sigParamChanged);
    cls_modbus_slaver.regist();

    jzbind::registFunction("initModbusMaster", true, initModbusMaster);
    jzbind::registFunction("initModbusSlaver", true, initModbusSlaver);

    JZNodeFactory::instance()->registNode(Node_modbusConfig, createJZNode<JZNodeModbusConfig>);

    JZNodeEditorManager::instance()->registCustomFunctionNode("initModbusMaster", Node_modbusConfig);
    JZNodeEditorManager::instance()->registCustomFunctionNode("initModbusSlaver", Node_modbusConfig);
}

void JZModuleModbus::unregist(JZScriptEnvironment *env)
{

}