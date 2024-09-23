#include <QHBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QStackedWidget>
#include <QCheckBox>
#include "JZModbusConfigDialog.h"
#include "JZBaseDialog.h"
#include "JZModbusTemplateDialog.h"

//ModbusAddDialog
class ModbusAddDialog : public JZBaseDialog
{
public:
    ModbusAddDialog(QWidget *widget)
        :JZBaseDialog(widget)
    {
        QFormLayout *l = new QFormLayout();
        l->setContentsMargins(0, 0, 0, 0);

        auto boxAddr = new QComboBox();
        boxAddr->addItems(JZModbusParam::addrTypeList());

        auto boxData = new QComboBox();
        boxData->addItems(JZModbusParam::dataTypeList());
        
        m_lineAddr = new QLineEdit();
        m_boxAddr = boxAddr;
        m_boxData = boxData;
        m_lineMemo = new QLineEdit();

        l->addRow("addr", m_lineAddr);
        l->addRow("addrType", boxAddr);
        l->addRow("dataType", boxData);
        l->addRow("memo", m_lineMemo);

        m_mainWidget->setLayout(l);
        setInfo(JZModbusParam());
    }

    void setInfo(JZModbusParam info)
    {
        m_lineAddr->setText(QString::number(info.addr));
        m_boxAddr->setCurrentIndex(info.addrType);
        m_boxData->setCurrentIndex(info.dataType);
        m_lineMemo->setText(info.memo);
    }

    JZModbusParam info()
    {
        JZModbusParam info;
        info.addr = m_lineAddr->text().toInt();
        info.addrType = m_boxAddr->currentIndex();
        info.dataType = m_boxData->currentIndex();
        info.memo = m_lineMemo->text();
        return info;
    }

    virtual bool onOk() override
    {
        return true;
    }

protected:
    QLineEdit *m_lineAddr;
    QLineEdit *m_lineMemo;
    QComboBox *m_boxAddr;
    QComboBox *m_boxData;
};

//ModbusStargeDialog
ModeStargeDialog::ModeStargeDialog(QWidget *widget)
    :JZBaseDialog(widget)
{
    QFormLayout *l = new QFormLayout();
    l->setContentsMargins(0, 0, 0, 0);

    m_boxRecv = new QCheckBox();
    m_boxRead = new QCheckBox();
    m_lineTime = new QLineEdit();
        
    l->addRow("recvNotify", m_boxRecv);
    l->addRow("autoRead", m_boxRead);
    l->addRow("autoReadTime", m_lineTime);

    m_mainWidget->setLayout(l);
    setInfo(JZModbusStrategy());
}

void ModeStargeDialog::setInfo(JZModbusStrategy info)
{
    m_lineTime->setText(QString::number(info.autoReadTime));
    m_boxRecv->setChecked(info.recvNotify);
    m_boxRead->setChecked(info.autoRead);        
}

JZModbusStrategy ModeStargeDialog::info()
{
    JZModbusStrategy info;
    info.autoReadTime = m_lineTime->text().toInt();
    info.recvNotify = m_boxRecv->isChecked();
    info.autoRead = m_boxRead->isChecked();
    return info;
}

bool ModeStargeDialog::onOk()
{
    return true;
}

JZModbusConfigDialog::JZModbusConfigDialog(QWidget *parent)
    :QDialog(parent)
{
    ui.setupUi(this);
    
    ui.boxType->addItem("RtuMaster", Modbus_rtuClient);
    ui.boxType->addItem("TcpMaster", Modbus_tcpClient);
    ui.boxType->addItem("RtuSlave", Modbus_rtuServer);
    ui.boxType->addItem("TcpSlave", Modbus_tcpServer);

    QStringList headers = { "地址","功能","类型","值", "策略","备注" };
    ui.tableModbus->setColumnCount(headers.size());
    ui.tableModbus->setHorizontalHeaderLabels(headers);    

    m_rtuSlave = 1;
    m_tcpSlave = 255;

    connect(ui.boxType, SIGNAL(currentIndexChanged(int)), this, SLOT(onBoxTypeChanged()));
    initComm();

    setConfig(JZModbusConfig());
}

JZModbusConfigDialog::~JZModbusConfigDialog()
{
}

void JZModbusConfigDialog::initComm()
{
    QHBoxLayout *l_stack = new QHBoxLayout();
    l_stack->setContentsMargins(0, 0, 0, 0);

    m_stack = new QStackedWidget();
    l_stack->addWidget(m_stack);
    ui.groupBox->setLayout(l_stack);

    {
        QFormLayout *l = new QFormLayout();
        l->setContentsMargins(0, 0, 0, 0);

        auto comboBoxCom = new QComboBox();
        comboBoxCom->addItem("COM1");
        comboBoxCom->addItem("COM2");
        comboBoxCom->addItem("COM3");
        comboBoxCom->addItem("COM4");
        comboBoxCom->addItem("COM5");
        comboBoxCom->addItem("COM6");
        comboBoxCom->addItem("COM7");
        comboBoxCom->addItem("COM8");
        comboBoxCom->setEditable(true);

        auto comboBoxBotelv = new QComboBox();
        comboBoxBotelv->addItem("9600");
        comboBoxBotelv->addItem("19200");
        comboBoxBotelv->addItem("38400");
        comboBoxBotelv->addItem("56000");
        comboBoxBotelv->addItem("57600");
        comboBoxBotelv->addItem("115200");
        comboBoxBotelv->setCurrentIndex(0);
        comboBoxBotelv->setEditable(true);

        auto comboBoxData = new QComboBox();
        comboBoxData->addItem("5");
        comboBoxData->addItem("6");
        comboBoxData->addItem("7");
        comboBoxData->addItem("8");
        comboBoxData->setCurrentIndex(3);

        auto comboBoxStop = new QComboBox();
        comboBoxStop->addItem("1");
        comboBoxStop->addItem("2");
        comboBoxStop->setCurrentIndex(0);

        auto comboBoxChk = new QComboBox();
        comboBoxChk->addItem("NONE无");
        comboBoxChk->addItem("EVEN偶");
        comboBoxChk->addItem("ODD奇");
        comboBoxChk->setCurrentIndex(0);
        
        l->addRow("串口号:", comboBoxCom);
        l->addRow("波特率:", comboBoxBotelv);
        l->addRow("数据位:", comboBoxData);
        l->addRow("停止位:", comboBoxStop);
        l->addRow("校验位:", comboBoxChk);

        m_comboBoxCom = comboBoxCom;
        m_comboBoxBotelv = comboBoxBotelv;
        m_comboBoxData = comboBoxData;
        m_comboBoxStop = comboBoxStop;
        m_comboBoxChk = comboBoxChk;

        QWidget *w = new QWidget();
        w->setLayout(l);
        m_stack->addWidget(w);
    }

    {
        QFormLayout *l = new QFormLayout();
        l->setContentsMargins(0, 0, 0, 0);

        auto line_ip = new QLineEdit();
        line_ip->setText("127.0.0.1");

        auto line_port = new QLineEdit();
        line_port->setText("502");
        
        l->addRow("ip:", line_ip);
        l->addRow("port:", line_port);
        m_lineIp = line_ip;
        m_linePort = line_port;

        QWidget *w = new QWidget();
        w->setLayout(l);
        m_stack->addWidget(w);
    }

    {
        QFormLayout *l = new QFormLayout();
        l->setContentsMargins(0, 0, 0, 0);

        auto line_port = new QLineEdit();
        line_port->setText("502");
        
        l->addRow("port:", line_port);
        m_linePortServer = line_port;

        QWidget *w = new QWidget();
        w->setLayout(l);
        m_stack->addWidget(w);
    }
}

void JZModbusConfigDialog::setCurrentModbusType(int type)
{
    int idx = ui.boxType->findData(type);
    ui.boxType->setCurrentIndex(idx);
}

int JZModbusConfigDialog::currentModbusType()
{
    return ui.boxType->currentData().toInt();
}

void JZModbusConfigDialog::setConfigMode(bool isServer)
{
    ui.boxType->blockSignals(true);
    ui.boxType->clear();
    if (isServer)
    {
        ui.boxType->addItem("RtuMaster", Modbus_rtuClient);
        ui.boxType->addItem("TcpMaster", Modbus_tcpClient);
    }
    else
    {
        ui.boxType->addItem("RtuSlave", Modbus_rtuServer);
        ui.boxType->addItem("TcpSlave", Modbus_tcpServer);
    }
    ui.boxType->blockSignals(false);        
}

void JZModbusConfigDialog::setConfig(const JZModbusConfig &cfg)
{            
    setCurrentModbusType(cfg.modbusType);
    if (cfg.isRtu())
    {
        ui.boxType->setCurrentIndex(0);
        m_rtuSlave = cfg.slave;
    }
    else
    {
        ui.boxType->setCurrentIndex(1);
        m_tcpSlave = cfg.slave;
    }

    int type = cfg.modbusType;
    if (type == Modbus_rtuClient || type == Modbus_rtuServer)
    {
        m_stack->setCurrentIndex(0);        
        ui.lineSlave->setText(QString::number(m_rtuSlave));
    }
    else if (type == Modbus_tcpClient)
    {
        m_stack->setCurrentIndex(1);        
        ui.lineSlave->setText(QString::number(m_tcpSlave));
    }
    else
    {
        m_stack->setCurrentIndex(2);        
        ui.lineSlave->setText(QString::number(m_tcpSlave));
    }
    ui.chkPlc->setChecked(cfg.plcMode);

    m_comboBoxCom->setCurrentText(cfg.portName);
    m_comboBoxBotelv->setCurrentText(QString::number(cfg.baud));
    m_comboBoxData->setCurrentIndex(cfg.dataBit);
    m_comboBoxChk->setCurrentIndex(cfg.parityBit);
    m_comboBoxStop->setCurrentIndex(cfg.stopBit);
    
    m_lineIp->setText(cfg.ip);
    m_linePort->setText(QString::number(cfg.port));
    m_linePortServer->setText(QString::number(cfg.port));

    m_map.clear();
    for (int i = 0; i < cfg.paramList.size(); i++)
        m_map.add(cfg.paramList[i]);

    m_strategyMap = cfg.strategyMap;
    updateTable();
}

JZModbusConfig JZModbusConfigDialog::config()
{    
    JZModbusConfig cfg;
    
    int type = currentModbusType();
    cfg.modbusType = type;
    cfg.slave = ui.lineSlave->text().toInt();    
    if (type == Modbus_rtuClient || type == Modbus_rtuServer)
    {
        cfg.portName = m_comboBoxCom->currentText();
        cfg.baud = m_comboBoxBotelv->currentText().toInt();
        cfg.dataBit = m_comboBoxData->currentIndex();
        cfg.parityBit = m_comboBoxChk->currentIndex();
        cfg.stopBit = m_comboBoxStop->currentIndex();
    }
    else if (type == Modbus_tcpClient)
    {
        cfg.ip = m_lineIp->text();
        cfg.port = m_linePort->text().toInt();
    }
    else
    {
        cfg.port = m_linePortServer->text().toInt();        
    }
    cfg.strategyMap = m_strategyMap;
    cfg.plcMode = ui.chkPlc->isChecked();

    auto param_list = m_map.paramList();
    for (int i = 0; i < param_list.size(); i++)
        cfg.paramList.push_back(*m_map.param(param_list[i]));

    return cfg;
}

void JZModbusConfigDialog::onBoxTypeChanged()
{
    int type = currentModbusType();
    if (type == Modbus_rtuClient || type == Modbus_rtuServer)
    {
        ui.lineSlave->setText(QString::number(m_rtuSlave));
        m_stack->setCurrentIndex(0);
    }
    else
    { 
        ui.lineSlave->setText(QString::number(m_tcpSlave));
        if (type == Modbus_tcpClient)
            m_stack->setCurrentIndex(1);
        else
            m_stack->setCurrentIndex(1);
    }
}

void JZModbusConfigDialog::on_btnAdd_clicked()
{
    ModbusAddDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    if (!m_map.add(dlg.info()))
    {
        QMessageBox::information(this, "", "地址已存在或参数重合");
        return;
    }
    updateTable();
}

void JZModbusConfigDialog::on_btnEdit_clicked()
{
    int row = ui.tableModbus->currentRow();
    if (row == -1)
        return;

    int addr = ui.tableModbus->item(row, 0)->data(Qt::UserRole).toInt();
    
    JZModbusParam old = *m_map.param(addr);
    ModbusAddDialog dlg(this);
    dlg.setInfo(old);
    if (dlg.exec() != QDialog::Accepted)
        return;
    
    m_map.remove(addr);
    if (!m_map.add(dlg.info()))
    {
        m_map.add(old);
        QMessageBox::information(this, "", "地址已存在或参数重合");
        return;
    }
    if (addr != dlg.info().addr)
        m_strategyMap.remove(addr);
    updateTable();
}

void JZModbusConfigDialog::on_btnRemove_clicked()
{
    int row = ui.tableModbus->currentRow();
    if (row == 0)
        return;

    int addr = ui.tableModbus->item(row, 0)->data(Qt::UserRole).toInt();
    m_map.remove(addr);
    m_strategyMap.remove(addr);
    updateTable();
}

void JZModbusConfigDialog::on_btnSelectTemplate_clicked()
{
    JZModbusTemplateDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    setConfig(dlg.config());
}

void JZModbusConfigDialog::on_btnClose_clicked()
{
    accept();
}

void JZModbusConfigDialog::updateTable()
{
    ui.tableModbus->clearContents();
    ui.tableModbus->blockSignals(true);

    auto paramList = m_map.paramList();
    qSort(paramList);

    auto addr_types = JZModbusParam::addrTypeList();
    auto data_types = JZModbusParam::dataTypeList();

    int proto_size = paramList.size();
    ui.tableModbus->setRowCount(proto_size);
    for (int i = 0; i < proto_size; i++)
    {
        auto proto = m_map.param(paramList[i]);
        int col = 0;
        QTableWidgetItem *itemAddr = new QTableWidgetItem();
        QTableWidgetItem *itemAddrType = new QTableWidgetItem();
        QTableWidgetItem *itemDataType = new QTableWidgetItem();
        QTableWidgetItem *itemMemo = new QTableWidgetItem();
        QTableWidgetItem *itemValue = new QTableWidgetItem();
        itemAddr->setText(QString::number(proto->addr));
        itemAddr->setData(Qt::UserRole,proto->addr);        
        itemAddr->setFlags(itemAddr->flags() & ~Qt::ItemIsEditable);        

        itemAddrType->setText(addr_types[proto->addrType]);
        itemAddrType->setFlags(itemAddr->flags() & ~Qt::ItemIsEditable);

        itemDataType->setText(data_types[proto->dataType]);
        itemDataType->setFlags(itemAddr->flags() & ~Qt::ItemIsEditable);

        itemValue->setText(proto->value.toString());

        itemMemo->setText(proto->memo);
        itemMemo->setFlags(itemAddr->flags() & ~Qt::ItemIsEditable);

        ui.tableModbus->setItem(i, 0, itemAddr);
        ui.tableModbus->setItem(i, 1, itemAddrType);
        ui.tableModbus->setItem(i, 2, itemDataType);
        ui.tableModbus->setItem(i, 3, itemValue);

        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout();
        layout->setMargin(3);

        QWidget *widget_strage = new QWidget();
        QHBoxLayout *layout2 = new QHBoxLayout();
        layout2->setMargin(3);
        widget_strage->setLayout(layout2);
        
        QPushButton *btnStrategy = new QPushButton("设置");
        btnStrategy->setProperty("addr", proto->addr);
        connect(btnStrategy, SIGNAL(clicked()), this, SLOT(onProtoStrategyClicked()));
        layout2->addWidget(btnStrategy);

        ui.tableModbus->setCellWidget(i, 4, widget_strage);

        ui.tableModbus->setItem(i, 5, itemMemo);
    }
    ui.tableModbus->blockSignals(false);
}

int JZModbusConfigDialog::indexOfRow(int addr)
{
    for (int i = 0; i < ui.tableModbus->rowCount(); i++)
    {
        int item_addr = ui.tableModbus->item(i, 0)->data(Qt::UserRole).toInt();
        if (item_addr == addr)
            return i;
    }
    return -1;
}

void JZModbusConfigDialog::onProtoStrategyClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int addr = btn->property("addr").toInt();

    ModeStargeDialog dlg(this);
    if(m_strategyMap.contains(addr))
        dlg.setInfo(m_strategyMap[addr]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_strategyMap[addr] = dlg.info();
}