#include "JZModuleCommEditor.h"
#include "JZModuleComm.h"
#include "JZEditorGlobal.h"
#include "JZRegExpHelp.h"
#include "JZNodeView.h"

//JZCommConfigDialog
JZCommConfigDialog::JZCommConfigDialog(QWidget *parent)
    :JZManagerPropertyDialog(parent)
{
    auto browser = m_editor->browser();
    connect(browser, &JZPropertyBrowser::valueChanged, this, &JZCommConfigDialog::onPropChanged);

    auto group = m_editor->addGroup("基本");    
    m_editor->addProp("名称", &m_config.name, group );

    QList<int> enmuList = { Comm_ModbusRtuClient, Comm_ModbusTcpClient };
    QStringList enumTextList = {"ModbusRtuClient" ,"ModbusTcpClient"};
    m_typeProp = m_editor->addPropIntEnum("类型", &m_config.commType, enmuList, enumTextList, group);

    auto prop_group = m_editor->addGroup("属性");

    QList<JZProperty*> modbus_rtu, modbus_tcp;

    QList<int> bit_order_value = { QDataStream::LittleEndian, QDataStream::BigEndian};
    QStringList bit_order_text = { "LittleEndian", "BigEndian"};
    JZProperty *bit_order = m_editor->addPropIntEnum("BitOrder", &m_config.modbus.bitOrder, bit_order_value, bit_order_text, prop_group);

    //rtu    
    QList<int> baud_value = { QSerialPort::Baud9600, QSerialPort::Baud19200, QSerialPort::Baud38400, 
        QSerialPort::Baud57600, QSerialPort::Baud115200, };
    QStringList baud_text = { "9600", "19200", "38400", "57600", "115200" };
    
    QList<int> dataBit_value = { 5,6,7,8 };
    QStringList dataBit_text = { "5", "6", "7", "8" };
    
    QList<int> parityBit_value = { QSerialPort::NoParity, QSerialPort::EvenParity, QSerialPort::OddParity };
    QStringList parityBit_text = { "No", "Even", "Odd" };

    QList<int> stopBit_value = { QSerialPort::OneStop, QSerialPort::TwoStop };
    QStringList stopBit_text = { "OneStop", "TwoStop" };

    modbus_rtu << bit_order;

    auto comm_group = m_editor->addGroup("通信");
    modbus_rtu << m_editor->addProp("PortName", &m_config.modbus.conn.portName, comm_group);
    modbus_rtu << m_editor->addPropIntEnum("Baud", &m_config.modbus.conn.baud, baud_value, baud_text, comm_group);
    modbus_rtu << m_editor->addPropIntEnum("DataBit", &m_config.modbus.conn.dataBit, dataBit_value, dataBit_text, comm_group);
    modbus_rtu << m_editor->addPropIntEnum("ParityBit", &m_config.modbus.conn.parityBit, parityBit_value, parityBit_text, comm_group);
    modbus_rtu << m_editor->addPropIntEnum("StopBit", &m_config.modbus.conn.stopBit, stopBit_value, stopBit_text, comm_group);

    //tcp
    modbus_tcp << bit_order;
    modbus_tcp << m_editor->addProp("Ip", &m_config.modbus.conn.ip, comm_group);
    modbus_tcp << m_editor->addProp("Port", &m_config.modbus.conn.port, comm_group);

    addPage(Comm_ModbusRtuClient, modbus_rtu);
    addPage(Comm_ModbusTcpClient, modbus_tcp);
}

void JZCommConfigDialog::setConfig(JZCommConfig cfg)
{
    m_config = cfg;
    m_editor->dataToUi();
    switchPage(m_config.commType);
}

JZCommConfig JZCommConfigDialog::getConfig() const
{
    return m_config;
}

void JZCommConfigDialog::accept()
{
    m_editor->uiToData();
    JZManagerPropertyDialog::accept();
}

//JZCommInitDialog
JZCommInitDialog::JZCommInitDialog(QWidget *parent)
    :JZNodeManagerDialog(parent)
{
    QStringList strListHeader = { "名称", "类型" };
    m_table->setColumnCount(strListHeader.size());
    m_table->setHorizontalHeaderLabels(strListHeader);

    m_camTypeList = QStringList{ "None","ModbusRtuClient","ModbusTcpClient" };
}

void JZCommInitDialog::setConfig(JZCommManagerConfig cfg)
{
    m_config = cfg;
    updateConfig();
}

JZCommManagerConfig JZCommInitDialog::config()
{
    return m_config;
}

void JZCommInitDialog::addConfig() 
{
    QStringList camera_list;
    for (int i = 0; i < m_config.commList.size(); i++)
        camera_list << m_config.commList[i].name;

    JZCommConfig cfg;
    cfg.name = JZRegExpHelp::uniqueString("comm", camera_list);
    cfg.commType = Comm_ModbusRtuClient;

    JZCommConfigDialog dlg(this);
    dlg.setConfig(cfg);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.commList << cfg;
    updateConfig();
}

void JZCommInitDialog::removeConfig(int index) 
{
    m_config.commList.removeAt(index);
    updateConfig();
}

void JZCommInitDialog::settingConfig(int index) 
{
    JZCommConfigDialog dlg(this);
    dlg.setConfig(m_config.commList[index]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.commList[index] = dlg.getConfig();
    updateConfig();
}

void JZCommInitDialog::updateConfig()
{
    m_table->setRowCount(m_config.commList.size());

    QTableWidget *item = new QTableWidget();    
    for (int i = 0; i < m_config.commList.size(); i++)
    {
        auto &cfg = m_config.commList[i];
        QTableWidgetItem *item = new QTableWidgetItem(cfg.name);
        m_table->setItem(i, 0, item);

        QTableWidgetItem *item_type = new QTableWidgetItem(m_camTypeList[cfg.commType]);
        m_table->setItem(i, 1, item_type);
    }
}

//JZCommInitItem    
JZCommInitItem::JZCommInitItem(JZNode *node)
    :JZNodeGraphItem(node)
{
    QPushButton *btnSet = new QPushButton("Setting");
    btnSet->connect(btnSet, &QPushButton::clicked, [this] {
        this->onSetClicked();
    });
    m_setting = createWidgetBlock(btnSet, true);
    m_setting->pri = 8;
}

void JZCommInitItem::onSetClicked()
{
    JZNodeCommInit *node = (JZNodeCommInit *)m_node;
    JZCommInitDialog dlg(editor());
    dlg.setConfig(node->config());
    if (dlg.exec() != QDialog::Accepted)
        return;

    QByteArray oldValue = saveNode();
    node->setConfig(dlg.config());
    QByteArray newValue = saveNode();
    if (newValue == oldValue)
        return;

    notifyPropChanged(oldValue);
}

//JZCommModbusRWItem
JZCommModbusRWItem::JZCommModbusRWItem(JZNode *node)
    :JZNodeGraphItem(node)
{    
    m_funcList = QStringList{ "Bit", "InputBit", "InputRegister", "Register" };    

    m_modbusFunc = createEditBlock("Func", JZParamEditInfo::createEnum(m_funcList));
    m_modbusDataType = createEditBlock("Type", JZParamEditInfo()); 
}

void JZCommModbusRWItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    auto in_list = m_node->paramInList();
    m_blocks[in_list[0]]->pri = Pri_user;
    m_modbusFunc->pri = Pri_user + 1;
    m_modbusDataType->pri = Pri_user + 2;
    for(int i = 1; i < in_list.size(); i++)
        m_blocks[in_list[i]]->pri = Pri_user + i + 3;
    
    auto node = dynamic_cast<JZNodeModbusRW*>(m_node);
    if (node->function() == Function_Bit || node->function() == Function_InputBit)
    {
        m_modbusDataType->isEditable = false;        
    }
    else
    {
        QList<int> type_list = { Type_int16, Type_uint16, Type_int, Type_uint, Type_float, Type_double };
        m_dataTypeList = editorEnvironment()->typeListToNameList(type_list);
        m_modbusDataType->isEditable = true;
        m_modbusDataType->edit = JZParamEditInfo::createEnum(m_dataTypeList);
    }
}

void JZCommModbusRWItem::setBlockValue(int pin, QString value)
{
    QByteArray buffer = saveNode();

    auto node = dynamic_cast<JZNodeModbusRW*>(m_node);
    if (pin == m_modbusFunc->id)
        node->setFunction(m_funcList.indexOf(value));
    else if (pin == m_modbusDataType->id)
        node->setDataType(value);
    else {
        Q_ASSERT(0);        
    }
    notifyPropChanged(buffer);
}

QString JZCommModbusRWItem::blockValue(int pin)
{    
    auto node = dynamic_cast<JZNodeModbusRW*>(m_node);
    if (pin == m_modbusFunc->id)
        return m_funcList[node->function()];
    else if (pin == m_modbusDataType->id)
        return node->dataType();
    else {
        Q_ASSERT(0);
        return QString();
    }        
}

//JZModuleCommEditorInit
void JZModuleCommEditorInit()
{
    auto inst = editorManager()->instance();

    inst->registLogicNode(Node_CommInit,"通信", CreateJZNodeGraphItem<JZCommInitItem>);
    inst->registLogicNode(Node_ModbusRead,"通信", CreateJZNodeGraphItem<JZCommModbusRWItem>);
    inst->registLogicNode(Node_ModbusWrite,"通信", CreateJZNodeGraphItem<JZCommModbusRWItem>);
    inst->registLogicNode(Node_TcpClientRead,"通信");
    inst->registLogicNode(Node_TcpClientWrite,"通信");
    inst->registLogicNode(Node_UdpRead,"通信");
    inst->registLogicNode(Node_UdpWrite,"通信");
    inst->registLogicNode(Node_SerialRead,"通信");
    inst->registLogicNode(Node_SerialWrite,"通信");
}