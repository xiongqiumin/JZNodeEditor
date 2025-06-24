#include <QToolButton>
#include "JZModbusSimulator.h"
#include "JZNodeUtils.h"
#include "JZModbusConfigDialog.h"
#include "jzModbus/JZModbusParam.h"

JZModBusSimulator::JZModBusSimulator()
{
    m_master = nullptr;
    m_slaver = nullptr;

    QWidget *tool = new QWidget();
    QHBoxLayout *l_tool = new QHBoxLayout();
    l_tool->setContentsMargins(0, 0, 0, 0);
    tool->setLayout(l_tool);

    QToolButton* btnStart = new QToolButton();
    QToolButton* btnStop = new QToolButton();
    btnStop->setEnabled(false);
    QToolButton* btnSetting = new QToolButton();
    l_tool->addWidget(btnStart);
    l_tool->addWidget(btnStop);
    l_tool->addWidget(btnSetting);
    l_tool->addStretch();
    
    m_btnStart = btnStart;
    m_btnStop = btnStop;
    m_btnSetting = btnSetting;

    QVBoxLayout *l = new QVBoxLayout();
    l->addWidget(tool);

    m_table = new QTableWidget();
    l->addWidget(m_table);
    this->setLayout(l);

    //connect
    btnStart->setIcon(QIcon(":/JZNodeEditor/Resources/icons/iconRun.png"));
    btnStop->setIcon(QIcon(":/JZNodeEditor/Resources/icons/iconStop.png"));
    btnSetting->setIcon(QIcon(":/JZNodeEditor/Resources/icons/iconSetting.png"));

    connect(btnStart, &QToolButton::clicked, this, &JZModBusSimulator::onSimulatorStart);
    connect(btnStop, &QToolButton::clicked, this, &JZModBusSimulator::onSimulatorStop);
    connect(btnSetting, &QToolButton::clicked, this, &JZModBusSimulator::onSimulatorSetting);

    QStringList headers = { "地址","功能","类型","值","操作","策略","备注" };
    m_table->setColumnCount(headers.size());
    m_table->setHorizontalHeaderLabels(headers);
    connect(m_table, &QTableWidget::itemChanged, this, &JZModBusSimulator::onItemChanged);    
}

JZModBusSimulator::~JZModBusSimulator()
{
    if (m_master)
    {
        m_master->close();
        delete m_master;
        m_master = nullptr;
    }

    if (m_slaver)
    {
        m_slaver->stopServer();
        delete m_slaver;
        m_slaver = nullptr;
    }
}

JZCommSimulatorType JZModBusSimulator::type()
{
    return Sim_Modbus;
}

bool JZModBusSimulator::isOpen()
{
    if (m_slaver)
        return m_slaver->isStart();
    else if (m_master)
        return m_master->isOpen();
    else
        return false;
}

bool JZModBusSimulator::open()
{
    if (m_slaver)
        return m_slaver->startServer();
    else if (m_master)
        return m_master->open();
    else
        return false;
}

void JZModBusSimulator::close()
{
    if (m_slaver)
        return m_slaver->stopServer();
    else if (m_master)
        return m_master->close();    
}

void JZModBusSimulator::setConfig(const QByteArray &buffer)
{
    m_config = JZNodeUtils::fromBuffer<JZModbusConfig>(buffer);
    initSimulator();
}

QByteArray JZModBusSimulator::getConfig()
{
    return JZNodeUtils::toBuffer(m_config);
}

void JZModBusSimulator::updateTable()
{
    QTableWidget *table = m_table;

    table->clearContents();
    table->blockSignals(true);

    JZModbusParamMap *map = nullptr;
    if (m_master)
        map = m_master->map();
    else
        map = m_slaver->map();

    auto paramList = map->paramList();
    qSort(paramList);

    auto addr_types = JZModbusParam::addrTypeList();
    auto data_types = JZModbusParam::dataTypeList();

    int proto_size = paramList.size();
    table->setRowCount(proto_size);
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
        itemAddr->setData(Qt::UserRole, proto->addr);
        itemAddr->setFlags(itemAddr->flags() & ~Qt::ItemIsEditable);

        itemAddrType->setText(addr_types[proto->addrType]);
        itemAddrType->setFlags(itemAddr->flags() & ~Qt::ItemIsEditable);

        itemDataType->setText(data_types[proto->dataType]);
        itemDataType->setFlags(itemAddr->flags() & ~Qt::ItemIsEditable);

        itemValue->setText(proto->value.toString());

        itemMemo->setText(proto->memo);
        itemMemo->setFlags(itemAddr->flags() & ~Qt::ItemIsEditable);

        table->setItem(i, 0, itemAddr);
        table->setItem(i, 1, itemAddrType);
        table->setItem(i, 2, itemDataType);
        table->setItem(i, 3, itemValue);

        QWidget *widget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout();
        layout->setMargin(3);

        QPushButton *btnRead = new QPushButton("读取");
        btnRead->setProperty("table", QVariant::fromValue(table));
        btnRead->setProperty("addr", proto->addr);
        connect(btnRead, SIGNAL(clicked()), this, SLOT(onProtoReadClicked()));
        layout->addWidget(btnRead);

        if (proto->addrType == Param_Coil || proto->addrType == Param_HoldingRegister)
        {
            QPushButton *btnWrite = new QPushButton("写入");
            btnWrite->setProperty("table", QVariant::fromValue(table));
            btnWrite->setProperty("addr", proto->addr);
            connect(btnWrite, SIGNAL(clicked()), this, SLOT(onProtoWriteClicked()));
            layout->addWidget(btnWrite);
        }

        widget->setLayout(layout);
        table->setCellWidget(i, 4, widget);

        QWidget *widget_strage = new QWidget();
        QHBoxLayout *layout2 = new QHBoxLayout();
        layout2->setMargin(3);
        widget_strage->setLayout(layout2);

        QPushButton *btnStrategy = new QPushButton("设置");
        btnStrategy->setProperty("table", QVariant::fromValue(table));
        connect(btnStrategy, SIGNAL(clicked()), this, SLOT(onProtoStrategyClicked()));
        layout2->addWidget(btnStrategy);

        table->setCellWidget(i, 5, widget_strage);

        table->setItem(i, 6, itemMemo);
    }
    table->blockSignals(false);
}


void JZModBusSimulator::onSimulatorStart()
{    
    startSimulator();
}

void JZModBusSimulator::onSimulatorStop()
{    
    stopSimulator();
}

void JZModBusSimulator::onSimulatorSetting()
{    
    JZModbusConfigDialog dlg(this);
    dlg.setConfig(m_config);
    if (dlg.exec() != JZModbusConfigDialog::Accepted)
        return;

    m_config = dlg.config();
    initSimulator();
}

void JZModBusSimulator::onItemChanged(QTableWidgetItem *item)
{
    auto table = item->tableWidget();    

    int row = item->row();
    int addr = table->item(row, 0)->data(Qt::UserRole).toInt();
    QVariant value = item->text();
    if (m_slaver)
        m_slaver->writeParam(addr, value);
    if (m_master)
        m_master->writeParam(addr, value);
}

int JZModBusSimulator::indexOfRow(int addr)
{
    for (int i = 0; i < m_table->rowCount(); i++)
    {
        int item_addr = m_table->item(i, 0)->data(Qt::UserRole).toInt();
        if (item_addr == addr)
            return i;
    }
    return -1;
}

void JZModBusSimulator::onParamChanged(int addr)
{
    QObject *obj = sender();
    
    QVariant v;
    if (m_master)
        v = m_master->readParam(addr);
    if (m_slaver)
        v = m_slaver->readParam(addr);

    int idx = indexOfRow(addr);
    m_table->item(idx, 3)->setText(v.toString());    
}

void JZModBusSimulator::onProtoReadClicked()
{
    auto btn = qobject_cast<QPushButton*>(sender());    
    if (m_master->isBusy())
        return;

    int addr = btn->property("addr").toInt();
    m_master->readRemoteParamAsync(addr);
}

void JZModBusSimulator::onProtoWriteClicked()
{
    auto btn = qobject_cast<QPushButton*>(sender());    
    if (m_master->isBusy())
        return;

    int addr = btn->property("addr").toInt();
    int row = indexOfRow(addr);

    QVariant v = m_table->item(row, 3)->text();
    m_master->writeRemoteParamAsync(addr, v);
}


void JZModBusSimulator::onProtoStrategyClicked()
{
    auto btn = qobject_cast<QPushButton*>(sender());
    int addr = btn->property("addr").toInt();

    ModeStargeDialog dlg(this);
    if (m_config.strategyMap.contains(addr))
        dlg.setInfo(m_config.strategyMap[addr]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    m_config.strategyMap[addr] = dlg.info();
    m_master->setStrategy(addr, dlg.info());
}


void JZModBusSimulator::updateStatus()
{
    m_btnStart->setEnabled(true);
    m_btnStop->setEnabled(true);
    m_btnSetting->setEnabled(true);
    if (isOpen())
    {
        m_btnStart->setEnabled(false);
        m_btnSetting->setEnabled(false);
    }
    else
    {
        m_btnStop->setEnabled(false);
    }
}


void JZModBusSimulator::startSimulator()
{
    auto &c = m_config;

    bool ret = false;
    int type = c.conn.modbusType;
    if (type == Modbus_rtuClient || type == Modbus_tcpClient)
        ret = m_master->open();
    else if (type == Modbus_rtuServer || type == Modbus_tcpServer)
        ret = m_slaver->startServer();

    if (!ret)
    {
        QMessageBox::information(this, "", "启动失败,请检查设置");
        return;
    }
    updateStatus();
}

void JZModBusSimulator::stopSimulator()
{
    auto &c = m_config;

    bool ret = false;
    int type = c.conn.modbusType;
    if (type == Modbus_rtuClient || type == Modbus_tcpClient)
        m_master->close();
    else if (type == Modbus_rtuServer || type == Modbus_tcpServer)
        m_slaver->stopServer();

    updateStatus();
}

void JZModBusSimulator::initSimulator()
{
    close();

    if (m_config.conn.modbusType == Modbus_rtuClient || m_config.conn.modbusType == Modbus_tcpClient)
    {
        m_master = new JZModbusMaster();
        modbusMasterSetConfig(m_master, &m_config);
        connect(m_master, &JZModbusMaster::sigParamChanged, this, &JZModBusSimulator::onParamChanged);
        m_master->setProperty("table", QVariant::fromValue(m_table));
    }
    else
    {
        m_table->hideColumn(4);
        m_table->hideColumn(5);
        m_slaver = new JZModbusSlaver();
        modbusSlaverSetConfig(m_slaver, &m_config);
        connect(m_slaver, &JZModbusSlaver::sigParamChanged, this, &JZModBusSimulator::onParamChanged);
        m_slaver->setProperty("table", QVariant::fromValue(m_table));
    }
    updateTable();
}