#include "JZCommWidget.h"
#include "JZRegExpHelp.h"

//JZCommConfigDialog
JZCommConfigDialog::JZCommConfigDialog(QWidget *parent)
    :JZPropertyDialog(parent)
{    
    auto group = m_editor->addGroup("基本");
    m_editor->addProp("名称", &m_name, group);

    QList<int> enmuList = { Comm_ModbusRtuClient, Comm_ModbusTcpClient };
    QStringList enumTextList = { "ModbusRtuClient" ,"ModbusTcpClient" };
    m_typeProp = m_editor->addPropIntEnum("类型", &m_type, enmuList, enumTextList, group);

    m_propGroup = m_editor->addGroup("属性");
    m_propGroup = m_editor->addGroup("通信");

    bit_order_value = QList<int> { QDataStream::LittleEndian, QDataStream::BigEndian};
    bit_order_text = QStringList{ "LittleEndian", "BigEndian"};

    baud_value = QList<int>{ QSerialPort::Baud9600, QSerialPort::Baud19200, QSerialPort::Baud38400, 
    QSerialPort::Baud57600, QSerialPort::Baud115200, };
    baud_text = QStringList{ "9600", "19200", "38400", "57600", "115200" };

    dataBit_value =  QList<int>{ 5,6,7,8 };
    dataBit_text = QStringList{ "5", "6", "7", "8" };

    parityBit_value = QList<int>{ QSerialPort::NoParity, QSerialPort::EvenParity, QSerialPort::OddParity };
    parityBit_text = QStringList{ "No", "Even", "Odd" };

    stopBit_value = QList<int>{ QSerialPort::OneStop, QSerialPort::TwoStop };
    stopBit_text = QStringList{ "OneStop", "TwoStop" };

    addModbusClient();
    addModbusServer();
    addTcpClient();
    addTcpServer();
    addUdp();
    addCom();
}

JZCommConfigDialog::~JZCommConfigDialog()
{
    qDeleteAll(m_config);
}

void JZCommConfigDialog::addModbusClient()
{
    JZCommModbusRtuClientConfig *config_rtu = new JZCommModbusRtuClientConfig();
    JZCommModbusTcpClientConfig *config_tcp = new JZCommModbusTcpClientConfig();

    int rtu_type,tcp_type;

    rtu_type = Comm_ModbusRtuClient;
    tcp_type = Comm_ModbusTcpClient;
    config_rtu->conn.modbusType = Modbus_rtuClient;
    config_tcp->conn.modbusType = Modbus_tcpClient;
     

    m_config[rtu_type] = new JZCommConfigEnum(config_rtu);
    m_config[tcp_type] = new JZCommConfigEnum(config_tcp);

    QList<JZProperty*> modbus_rtu, modbus_tcp;

    //rtu    
    JZProperty *bit_order_rtu = m_editor->addPropIntEnum("BitOrder", &config_rtu->bitOrder, bit_order_value, bit_order_text, m_propGroup);

    modbus_rtu << bit_order_rtu;    
    modbus_rtu << m_editor->addProp("PortName", &config_rtu->conn.portName, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("Baud", &config_rtu->conn.baud, baud_value, baud_text, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("DataBit", &config_rtu->conn.dataBit, dataBit_value, dataBit_text, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("ParityBit", &config_rtu->conn.parityBit, parityBit_value, parityBit_text, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("StopBit", &config_rtu->conn.stopBit, stopBit_value, stopBit_text, m_propGroup);

    //tcp
    JZProperty *bit_order_tcp = m_editor->addPropIntEnum("ByteOrder", &config_tcp->byteOrder, bit_order_value, bit_order_text, m_propGroup);
    modbus_tcp << bit_order_tcp;
    modbus_tcp << m_editor->addProp("Ip", &config_tcp->conn.ip, m_propGroup);
    modbus_tcp << m_editor->addProp("Port", &config_tcp->conn.port, m_propGroup);

    addPage(rtu_type, modbus_rtu);
    addPage(tcp_type, modbus_tcp);
}

void JZCommConfigDialog::addModbusServer()
{
    JZCommModbusRtuServerConfig* config_rtu = new JZCommModbusRtuServerConfig();
    JZCommModbusTcpServerConfig* config_tcp = new JZCommModbusTcpServerConfig();

    int rtu_type, tcp_type;
    rtu_type = Comm_ModbusRtuServer;
    tcp_type = Comm_ModbusTcpServer;
    config_rtu->conn.modbusType = Modbus_rtuServer;
    config_tcp->conn.modbusType = Modbus_tcpServer;

    m_config[rtu_type] = new JZCommConfigEnum(config_rtu);
    m_config[tcp_type] = new JZCommConfigEnum(config_tcp);

    QList<JZProperty*> modbus_rtu, modbus_tcp;

    //rtu    
    JZProperty* bit_order_rtu = m_editor->addPropIntEnum("BitOrder", &config_rtu->bitOrder, bit_order_value, bit_order_text, m_propGroup);

    modbus_rtu << bit_order_rtu;
    modbus_rtu << m_editor->addProp("PortName", &config_rtu->conn.portName, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("Baud", &config_rtu->conn.baud, baud_value, baud_text, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("DataBit", &config_rtu->conn.dataBit, dataBit_value, dataBit_text, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("ParityBit", &config_rtu->conn.parityBit, parityBit_value, parityBit_text, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("StopBit", &config_rtu->conn.stopBit, stopBit_value, stopBit_text, m_propGroup);

    //tcp
    JZProperty* bit_order_tcp = m_editor->addPropIntEnum("BitOrder", &config_tcp->bitOrder, bit_order_value, bit_order_text, m_propGroup);
    modbus_tcp << bit_order_tcp;
    //modbus_tcp << m_editor->addProp("Ip", &config_tcp->conn.ip, m_propGroup);
    modbus_tcp << m_editor->addProp("Port", &config_tcp->conn.port, m_propGroup);

    addPage(rtu_type, modbus_rtu);
    addPage(tcp_type, modbus_tcp);
}

void JZCommConfigDialog::addTcpClient()
{
    JZCommTcpClientConfig *config = new JZCommTcpClientConfig();
    m_config[Comm_TcpClient] = new JZCommConfigEnum(config);

    QList<JZProperty*> prop_list;
    prop_list << m_editor->addProp("Ip", &config->ip, m_propGroup);
    prop_list << m_editor->addProp("Port", &config->port, m_propGroup);
    addPage(Comm_TcpClient, prop_list);
}

void JZCommConfigDialog::addTcpServer()
{
    JZCommTcpServerConfig *config = new JZCommTcpServerConfig();
    m_config[Comm_TcpServer] = new JZCommConfigEnum(config);

    QList<JZProperty*> prop_list;
    prop_list << m_editor->addProp("Port", &config->port, m_propGroup);
    addPage(Comm_TcpServer, prop_list);
}

void JZCommConfigDialog::addUdp()
{
    JZCommUdpConfig * config = new JZCommUdpConfig();
    m_config[Comm_Udp] = new JZCommConfigEnum(config);

    QList<JZProperty*> prop_list_server;
    prop_list_server << m_editor->addProp("Port", &config->port, m_propGroup);
    addPage(Comm_Udp, prop_list_server);
}

void JZCommConfigDialog::addCom()
{
    JZSerialPortConfig *config = new JZSerialPortConfig();
    m_config[Comm_SerialPort] = new JZCommConfigEnum(config);

    QList<JZProperty*> prop_list;
    prop_list << m_editor->addProp("PortName", &config->portName, m_propGroup);
    prop_list << m_editor->addPropIntEnum("Baud", &config->baud, baud_value, baud_text, m_propGroup);
    prop_list << m_editor->addPropIntEnum("DataBit", &config->dataBit, dataBit_value, dataBit_text, m_propGroup);
    prop_list << m_editor->addPropIntEnum("ParityBit", &config->parityBit, parityBit_value, parityBit_text, m_propGroup);
    prop_list << m_editor->addPropIntEnum("StopBit", &config->stopBit, stopBit_value, stopBit_text, m_propGroup);
    addPage(Comm_SerialPort, prop_list);
}

void JZCommConfigDialog::setConfig(JZCommConfigEnum cfg)
{    
    JZModuleConfigFactory<JZCommConfig>::instance()->copyTo(cfg.data(), m_config[cfg->type]->data());
    m_type = cfg->type;
    m_name = cfg->name;
    m_editor->dataToUi();
    switchPage(m_type);
}

JZCommConfigEnum JZCommConfigDialog::getConfig() const
{
    JZCommConfigEnum ptr = *m_config[m_type];
    ptr->name = m_name;
    return ptr;
}

void JZCommConfigDialog::accept()
{
    m_editor->uiToData();
    JZPropertyDialog::accept();
}


//JZCommConfigWidget
JZCommConfigWidget::JZCommConfigWidget(QWidget *parent)
    :JZModuleConfigWidget(parent)
{
    QStringList strListHeader = { "名称", "类型" };
    m_table->setColumnCount(strListHeader.size());
    m_table->setHorizontalHeaderLabels(strListHeader);

    m_camTypeList = QStringList{ "None","ModbusRtuClient","ModbusTcpClient" };
}

void JZCommConfigWidget::setConfig(JZCommManagerConfig cfg)
{
    m_config = cfg;
    updateConfig();
}

JZCommManagerConfig JZCommConfigWidget::config()
{
    return m_config;
}

void JZCommConfigWidget::addConfig() 
{
    QStringList camera_list;
    for (int i = 0; i < m_config.commList.size(); i++)
        camera_list << m_config.commList[i]->name;

    JZCommModbusTcpClientConfig *cfg = new JZCommModbusTcpClientConfig();
    cfg->name = JZRegExpHelp::uniqueString(cfg->name, camera_list);    

    JZCommConfigDialog dlg(this);
    dlg.setConfig(JZCommConfigEnum(cfg));
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.commList << dlg.getConfig();
    updateConfig();
    emit sigCommChanged();
}

void JZCommConfigWidget::removeConfig(int index) 
{
    m_config.commList.removeAt(index);
    updateConfig();
    emit sigCommChanged();
}

void JZCommConfigWidget::settingConfig(int index) 
{
    JZCommConfigDialog dlg(this);
    dlg.setConfig(m_config.commList[index]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.commList[index] = dlg.getConfig();
    updateConfig();
    emit sigCommChanged();
}

void JZCommConfigWidget::updateConfig()
{
    m_table->setRowCount(m_config.commList.size());

    QTableWidget *item = new QTableWidget();    
    for (int i = 0; i < m_config.commList.size(); i++)
    {
        auto &cfg = m_config.commList[i];
        QTableWidgetItem *item = new QTableWidgetItem(cfg->name);
        m_table->setItem(i, 0, item);

        QTableWidgetItem *item_type = new QTableWidgetItem(m_camTypeList[cfg->type]);
        m_table->setItem(i, 1, item_type);
    }
}