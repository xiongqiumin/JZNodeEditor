﻿#include <QHBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QStackedWidget>
#include <QCheckBox>
#include "JZModbusSimulator.h"
#include "JZBaseDialog.h"

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

//ModbusSettingDialog
class ModbusSettingDialog : public JZBaseDialog
{
public:
    ModbusSettingDialog(QWidget *widget)
        :JZBaseDialog(widget)
    {
        m_stack = new QStackedWidget();
        setCentralWidget(m_stack);

        {
            QFormLayout *l = new QFormLayout();
            l->setContentsMargins(0, 0, 0, 0);

            comboBoxCom = new QComboBox();
            comboBoxCom->addItem("COM1");
            comboBoxCom->addItem("COM2");
            comboBoxCom->addItem("COM3");
            comboBoxCom->addItem("COM4");
            comboBoxCom->addItem("COM5");
            comboBoxCom->addItem("COM6");
            comboBoxCom->addItem("COM7");
            comboBoxCom->addItem("COM8");

            comboBoxBotelv = new QComboBox();
            comboBoxBotelv->addItem("9600");
            comboBoxBotelv->addItem("19200");
            comboBoxBotelv->addItem("38400");
            comboBoxBotelv->addItem("56000");
            comboBoxBotelv->addItem("57600");
            comboBoxBotelv->addItem("115200");
            comboBoxBotelv->setCurrentIndex(0);           

            comboBoxData = new QComboBox();
            comboBoxData->addItem("5");
            comboBoxData->addItem("6");
            comboBoxData->addItem("7");
            comboBoxData->addItem("8");
            comboBoxData->setCurrentIndex(3);

            comboBoxStop = new QComboBox();
            comboBoxStop->addItem("1");
            comboBoxStop->addItem("2");
            comboBoxStop->setCurrentIndex(0);

            comboBoxChk = new QComboBox();
            comboBoxChk->addItem("NONE无");
            comboBoxChk->addItem("EVEN偶");
            comboBoxChk->addItem("ODD奇");            
            comboBoxChk->setCurrentIndex(0);

            l->addRow("串口号:", comboBoxCom);
            l->addRow("波特率:", comboBoxBotelv);
            l->addRow("数据位:", comboBoxData);
            l->addRow("停止位:", comboBoxStop);
            l->addRow("校验位:", comboBoxChk);

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
            lineIp = line_ip;
            linePort = line_port;

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
            linePortServer = line_port;

            QWidget *w = new QWidget();
            w->setLayout(l);
            m_stack->addWidget(w);
        }
    }

    virtual bool onOk() override
    {
        return true;
    }

    QStackedWidget *m_stack;
    QComboBox *comboBoxCom, *comboBoxBotelv, *comboBoxData, *comboBoxStop, *comboBoxChk;
    QLineEdit *lineIp, *linePort, *linePortServer;
};


//ModbusStargeDialog
class ModeStargeDialog : public JZBaseDialog
{
public:
    ModeStargeDialog(QWidget *widget)
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

    void setInfo(JZModbusStrategy info)
    {
        m_lineTime->setText(QString::number(info.autoReadTime));
        m_boxRecv->setChecked(info.recvNotify);
        m_boxRead->setChecked(info.autoRead);        
    }

    JZModbusStrategy info()
    {
        JZModbusStrategy info;
        info.autoReadTime = m_lineTime->text().toInt();
        info.recvNotify = m_boxRecv->isChecked();
        info.autoRead = m_boxRead->isChecked();
        return info;
    }

    virtual bool onOk() override
    {
        return true;
    }

protected:    
    QCheckBox *m_boxRead;
    QCheckBox *m_boxRecv;
    QLineEdit *m_lineTime;
};

JZModbusSimulator::JZModbusSimulator(QWidget *parent)
    :QMainWindow(parent)
{
    ui.setupUi(this);

    m_baudRateList << QSerialPort::Baud9600 << QSerialPort::Baud19200 << QSerialPort::Baud38400 << QSerialPort::Baud57600 << QSerialPort::Baud115200;
    m_dataBitsList << QSerialPort::Data5 << QSerialPort::Data6 << QSerialPort::Data7 << QSerialPort::Data8;
    m_stopBitsList << QSerialPort::OneStop << QSerialPort::TwoStop;
    m_parityList << QSerialPort::NoParity << QSerialPort::EvenParity << QSerialPort::OddParity;

    m_baud = 0;
    m_dataBit = 3;
    m_parityBit = 0;
    m_stopBit= 0;
    
    m_portName = "COM1";
    m_ip = "127.0.0.1";
    m_port = m_serverPort = 502;

    m_run = false;
    ui.boxType->addItem("RtuClient");
    ui.boxType->addItem("TcpClient");
    ui.boxType->addItem("RtuServer");    
    ui.boxType->addItem("TcpServer");    

    QStringList headers = { "地址","功能","类型","值","操作","策略","备注" };
    ui.tableModbus->setColumnCount(headers.size());
    ui.tableModbus->setHorizontalHeaderLabels(headers);    
    connect(ui.tableModbus, &QTableWidget::itemChanged, this, &JZModbusSimulator::onItemChanged);

    connect(ui.boxType, SIGNAL(currentIndexChanged(int)), this, SLOT(onBoxTypeChanged()));

    m_master = nullptr;
    m_slaver = nullptr;
    init();
}

JZModbusSimulator::~JZModbusSimulator()
{
    close();
}

void JZModbusSimulator::init()
{
    JZModbusParamMap map;
    if (m_master)
    {
        map = *m_master->map();
        m_master->close();
        m_master->deleteLater();
        m_master = nullptr;
    }

    if (m_slaver)
    {
        map = *m_slaver->map();
        m_slaver->stopSever();
        m_slaver->deleteLater();
        m_slaver = nullptr;
    }

    int type = ui.boxType->currentIndex();
    if (type == 0 || type == 1)
    {
        m_master = new JZModbusMaster();
        connect(m_master, &JZModbusMaster::sigParamChanged, this, &JZModbusSimulator::onParamChanged);
        *m_master->map() = map;
    }
    else
    {
        m_slaver = new JZModbusSlaver();
        connect(m_slaver, &JZModbusSlaver::sigParamChanged, this, &JZModbusSimulator::onParamChanged);
        *m_slaver->map() = map;
    }
}

JZModbusParamMap *JZModbusSimulator::mapping()
{
    if (m_master)
        return m_master->map();
    else
        return m_slaver->map();
}

void JZModbusSimulator::onBoxTypeChanged()
{
    init();
    updateStatus();
}

void JZModbusSimulator::on_btnSetting_clicked()
{
    int type = ui.boxType->currentIndex();
    ModbusSettingDialog dlg(this);
    if (type == 0 || type == 2)
    {
        dlg.m_stack->setCurrentIndex(0);

        dlg.comboBoxCom->setCurrentText(m_portName);
        dlg.comboBoxBotelv->setCurrentIndex(m_baud);
        dlg.comboBoxData->setCurrentIndex(m_dataBit);
        dlg.comboBoxChk->setCurrentIndex(m_parityBit);
        dlg.comboBoxStop->setCurrentIndex(m_stopBit);
    }
    else if (type == 1)
    {
       dlg.m_stack->setCurrentIndex(1);

       dlg.lineIp->setText(m_ip);
       dlg.linePort->setText(QString::number(m_port));
    }
    else 
    {
        dlg.m_stack->setCurrentIndex(2);

        dlg.linePortServer->setText(QString::number(m_serverPort));
    }
    if (dlg.exec() != QDialog::Accepted)
        return;

    if (type == 0 || type == 2)
    {
        m_portName = dlg.comboBoxCom->currentText();
        m_baud = dlg.comboBoxBotelv->currentIndex();
        m_dataBit = dlg.comboBoxData->currentIndex();
        m_parityBit = dlg.comboBoxChk->currentIndex();
        m_stopBit = dlg.comboBoxStop->currentIndex();
    }
    else if (type == 1)
    {
        m_ip = dlg.lineIp->text();
        m_port = dlg.linePort->text().toInt();
    }
    else
    {
        m_serverPort = dlg.linePortServer->text().toInt();
    }
}

void JZModbusSimulator::on_btnStart_clicked()
{    
    int type = ui.boxType->currentIndex();
    if (!m_run)
    {
        auto baud = m_baudRateList[m_baud];
        auto dataBit = m_dataBitsList[m_dataBit];
        auto parityBit = m_parityList[m_parityBit];
        auto stopBit = m_stopBitsList[m_stopBit];
        
        bool ret = false;
        int slave = ui.lineSlave->text().toInt();
        if (type == 0 || type == 1)
        {                        
            if (type == 0)
            {
                ret = m_master->openRtu(m_portName, baud, dataBit, stopBit, parityBit);
                m_master->setSlave(slave);
            }
            else
                ret = m_master->openTcp(m_ip,m_port);

            auto it = m_strategy.begin();
            while (it != m_strategy.end())
            {
                m_master->setStrategy(it.key(), it.value());
                it++;
            }            
        }
        else if (type == 2 || type == 3)
        {                        
            m_slaver->initMapping();
            if (type == 2)
            {
                ret = m_slaver->startRtuServer(m_portName, baud, dataBit, stopBit, parityBit);
                m_slaver->setSlave(slave);
            }
            else
                ret = m_slaver->startTcpServer(m_port);            
        }
        if (!ret)
        {
            QMessageBox::information(this, "", ui.btnStart->text() + "失败,请检查设置");
            return;
        }

        m_run = true;
        updateStatus();
    }
    else
    {
        if (m_master)
            m_master->close();
        if (m_slaver)
            m_slaver->stopSever();

        m_run = false;
        updateStatus();
    }
}

void JZModbusSimulator::on_btnAdd_clicked()
{
    ModbusAddDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    if (!mapping()->add(dlg.info()))
    {
        QMessageBox::information(this, "", "地址已存在或参数重合");
        return;
    }
    updateTable();
}

void JZModbusSimulator::on_btnEdit_clicked()
{
    int row = ui.tableModbus->currentRow();
    if (row == -1)
        return;

    int addr = ui.tableModbus->item(row, 0)->data(Qt::UserRole).toInt();

    auto map = mapping();
    JZModbusParam old = *map->param(addr);
    ModbusAddDialog dlg(this);
    dlg.setInfo(old);
    if (dlg.exec() != QDialog::Accepted)
        return;
    
    map->remove(addr);    
    if (!map->add(dlg.info()))
    {
        map->add(old);
        QMessageBox::information(this, "", "地址已存在或参数重合");
        return;
    }
    if (addr != dlg.info().addr)
        m_strategy.remove(addr);
    updateTable();
}

void JZModbusSimulator::on_btnRemove_clicked()
{
    int row = ui.tableModbus->currentRow();
    if (row == 0)
        return;

    int addr = ui.tableModbus->item(row, 0)->data(Qt::UserRole).toInt();
    mapping()->remove(addr);
    m_strategy.remove(addr);
    updateTable();
}

void JZModbusSimulator::updateStatus()
{
    int type = ui.boxType->currentIndex();
    if (type == 0 || type == 1)
    {
        ui.tableModbus->setColumnHidden(4, false);
        ui.tableModbus->setColumnHidden(5, false);
        ui.btnStart->setText(m_run? "关闭":"连接");
    }
    else
    {
        ui.tableModbus->setColumnHidden(4, true);
        ui.tableModbus->setColumnHidden(5, true);
        ui.btnStart->setText(m_run? "停止":"启动");
    }    

    ui.boxType->setEnabled(!m_run);
    ui.btnSetting->setEnabled(!m_run);
    ui.btnAdd->setEnabled(!m_run);
    ui.btnEdit->setEnabled(!m_run);
    ui.btnRemove->setEnabled(!m_run);    
}

void JZModbusSimulator::updateTable()
{
    ui.tableModbus->clearContents();
    ui.tableModbus->blockSignals(true);

    auto map = mapping();
    auto paramList = map->paramList();
    qSort(paramList);

    auto addr_types = JZModbusParam::addrTypeList();
    auto data_types = JZModbusParam::dataTypeList();

    int proto_size = paramList.size();
    ui.tableModbus->setRowCount(proto_size);
    for (int i = 0; i < proto_size; i++)
    {
        auto proto = map->param(paramList[i]);
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

        QPushButton *btnRead = new QPushButton("读取");
        btnRead->setProperty("addr", proto->addr);
        connect(btnRead, SIGNAL(clicked()), this, SLOT(onProtoReadClicked()));
        layout->addWidget(btnRead);

        QPushButton *btnWrite = new QPushButton("写入");
        btnWrite->setProperty("addr", proto->addr);
        connect(btnWrite, SIGNAL(clicked()), this, SLOT(onProtoWriteClicked()));
        layout->addWidget(btnWrite);
        if (proto->addrType == Param_DiscreteInput || proto->addrType == Param_InputRegister)
            btnWrite->setEnabled(false);

        widget->setLayout(layout);
        ui.tableModbus->setCellWidget(i, 4, widget);

        QWidget *widget_strage = new QWidget();
        QHBoxLayout *layout2 = new QHBoxLayout();
        layout2->setMargin(3);
        widget_strage->setLayout(layout2);
        
        QPushButton *btnStrategy = new QPushButton("设置");
        btnStrategy->setProperty("addr", proto->addr);
        connect(btnStrategy, SIGNAL(clicked()), this, SLOT(onProtoStrategyClicked()));
        layout2->addWidget(btnStrategy);

        ui.tableModbus->setCellWidget(i, 5, widget_strage);

        ui.tableModbus->setItem(i, 6, itemMemo);
    }
    ui.tableModbus->blockSignals(false);
}

void JZModbusSimulator::onItemChanged(QTableWidgetItem *item)
{
    int row = item->row();
    int addr = ui.tableModbus->item(row, 0)->data(Qt::UserRole).toInt();
    QVariant value = item->text();
    if (m_slaver)           
        m_slaver->writeParam(addr, value);
    if (m_master)
        m_master->writeParam(addr, value);
}

int JZModbusSimulator::indexOfRow(int addr)
{
    for (int i = 0; i < ui.tableModbus->rowCount(); i++)
    {
        int item_addr = ui.tableModbus->item(i, 0)->data(Qt::UserRole).toInt();
        if (item_addr == addr)
            return i;
    }
    return -1;
}

void JZModbusSimulator::onParamChanged(int addr)
{
    QVariant v;
    if (m_master)
        v = m_master->readParam(addr);
    if (m_slaver)
        v = m_slaver->readParam(addr);

    int idx = indexOfRow(addr);
    ui.tableModbus->item(idx, 3)->setText(v.toString());
}

void JZModbusSimulator::onProtoReadClicked()
{
    if (m_master->isBusy())
        return;

    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int addr = btn->property("addr").toInt();
    m_master->readRemoteParamAsync(addr);
}

void JZModbusSimulator::onProtoWriteClicked()
{
    if (m_master->isBusy())
        return;

    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int addr = btn->property("addr").toInt();
    int row = indexOfRow(addr);
    
    QVariant v = ui.tableModbus->item(row, 3)->text();        
    m_master->writeRemoteParamAsync(addr,v);
}

void JZModbusSimulator::onProtoStrategyClicked()
{
    if (m_master->isOpen())
        return;

    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int addr = btn->property("addr").toInt();

    ModeStargeDialog dlg(this);
    if(m_strategy.contains(addr))
        dlg.setInfo(m_strategy[addr]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_strategy[addr] = dlg.info();
}