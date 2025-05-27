#include "JZCommWidget.h"
#include "JZRegExpHelp.h"

//JZCommConfigDialog
JZCommConfigDialog::JZCommConfigDialog(QWidget *parent)
    :JZPropertyDialog(parent)
{
    auto browser = m_editor->browser();
    connect(browser, &JZPropertyBrowser::valueChanged, this, &JZCommConfigDialog::onPropChanged);

    auto group = m_editor->addGroup("基本");
    m_editor->addProp("名称", &m_name, group);

    QList<int> enmuList = { Comm_ModbusRtuClient, Comm_ModbusTcpClient };
    QStringList enumTextList = { "ModbusRtuClient" ,"ModbusTcpClient" };
    m_typeProp = m_editor->addPropIntEnum("类型", &m_type, enmuList, enumTextList, group);

    m_propGroup = m_editor->addGroup("属性");
    m_propGroup = m_editor->addGroup("通信");

    addModbusClient();
    addModbusServer();
    addTcpClient();
    addTcpServer();
    addUdp();
    addCom();
}

void JZCommConfigDialog::addModbusClient()
{
    JZCommModbusClientConfig *config_rtu = new JZCommModbusClientConfig();
    config_rtu->conn.modbusType = Modbus_rtuClient;

    JZCommModbusClientConfig *config_tcp = new JZCommModbusClientConfig();
    config_tcp->conn.modbusType = Modbus_tcpClient;

    m_config[Comm_ModbusRtuClient] = JZCommConfigPtr(config_rtu);
    m_config[Comm_ModbusTcpClient] = JZCommConfigPtr(config_tcp);

    QList<JZProperty*> modbus_rtu, modbus_tcp;

    //rtu    
    QList<int> bit_order_value = { QDataStream::LittleEndian, QDataStream::BigEndian};
    QStringList bit_order_text = { "LittleEndian", "BigEndian"};
    JZProperty *bit_order_rtu = m_editor->addPropIntEnum("BitOrder", &config_rtu->bitOrder, bit_order_value, bit_order_text, m_propGroup);
    
    QList<int> baud_value = { QSerialPort::Baud9600, QSerialPort::Baud19200, QSerialPort::Baud38400, 
        QSerialPort::Baud57600, QSerialPort::Baud115200, };
    QStringList baud_text = { "9600", "19200", "38400", "57600", "115200" };
    
    QList<int> dataBit_value = { 5,6,7,8 };
    QStringList dataBit_text = { "5", "6", "7", "8" };
    
    QList<int> parityBit_value = { QSerialPort::NoParity, QSerialPort::EvenParity, QSerialPort::OddParity };
    QStringList parityBit_text = { "No", "Even", "Odd" };

    QList<int> stopBit_value = { QSerialPort::OneStop, QSerialPort::TwoStop };
    QStringList stopBit_text = { "OneStop", "TwoStop" };

    modbus_rtu << bit_order_rtu;    
    modbus_rtu << m_editor->addProp("PortName", &config_rtu->conn.portName, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("Baud", &config_rtu->conn.baud, baud_value, baud_text, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("DataBit", &config_rtu->conn.dataBit, dataBit_value, dataBit_text, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("ParityBit", &config_rtu->conn.parityBit, parityBit_value, parityBit_text, m_propGroup);
    modbus_rtu << m_editor->addPropIntEnum("StopBit", &config_rtu->conn.stopBit, stopBit_value, stopBit_text, m_propGroup);

    //tcp
    JZProperty *bit_order_tcp = m_editor->addPropIntEnum("BitOrder", &config_tcp->bitOrder, bit_order_value, bit_order_text, m_propGroup);
    modbus_tcp << bit_order_tcp;
    modbus_tcp << m_editor->addProp("Ip", &config_tcp->conn.ip, m_propGroup);
    modbus_tcp << m_editor->addProp("Port", &config_tcp->conn.port, m_propGroup);

    addPage(Comm_ModbusRtuClient, modbus_rtu);
    addPage(Comm_ModbusTcpClient, modbus_tcp);
}

void JZCommConfigDialog::addModbusServer()
{
}

void JZCommConfigDialog::addTcpClient()
{
}

void JZCommConfigDialog::addTcpServer()
{
}

void JZCommConfigDialog::addUdp()
{
}

void JZCommConfigDialog::addCom()
{
}

void JZCommConfigDialog::setConfig(JZCommConfigPtr cfg)
{    
    JZModuleConfigFactory<JZCommConfig>::instance()->copyTo(cfg.data(), m_config[cfg->type].data());
    m_type = cfg->type;
    m_name = cfg->name;
    m_editor->dataToUi();
    switchPage(m_type);
}

JZCommConfigPtr JZCommConfigDialog::getConfig() const
{
    JZCommConfigPtr ptr = m_config[m_type];
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

    JZCommModbusClientConfig *cfg = new JZCommModbusClientConfig();
    cfg->name = JZRegExpHelp::uniqueString("comm", camera_list);    

    JZCommConfigDialog dlg(this);
    dlg.setConfig(JZCommConfigPtr(cfg));
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.commList << dlg.getConfig();
    updateConfig();
}

void JZCommConfigWidget::removeConfig(int index) 
{
    m_config.commList.removeAt(index);
    updateConfig();
}

void JZCommConfigWidget::settingConfig(int index) 
{
    JZCommConfigDialog dlg(this);
    dlg.setConfig(m_config.commList[index]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.commList[index] = dlg.getConfig();
    updateConfig();
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