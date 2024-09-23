#include <QHBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QStackedWidget>
#include <QCheckBox>
#include <QSplitter>
#include <QMenuBar>
#include <QMdiSubWindow>
#include <QToolButton>
#include <QWindowStateChangeEvent>
#include <QFileDialog>
#include <QFile>
#include "JZModbusSimulator.h"
#include "JZModbusConfigDialog.h"


class SimulatorWidget : public QWidget
{
public:
    SimulatorWidget()
    {
        QWidget *tool = new QWidget();
        QHBoxLayout *l_tool = new QHBoxLayout();
        l_tool->setContentsMargins(0, 0, 0, 0);
        tool->setLayout(l_tool);

        btnStart = new QToolButton();
        btnStop = new QToolButton();
        btnStop->setEnabled(false);
        btnSetting = new QToolButton();
        l_tool->addWidget(btnStart);
        l_tool->addWidget(btnStop);
        l_tool->addWidget(btnSetting);
        l_tool->addStretch();        

        btnStart->setIcon(QIcon(":/JZNodeEditor/Resources/icons/iconRun.png"));
        btnStop->setIcon(QIcon(":/JZNodeEditor/Resources/icons/iconStop.png"));
        btnSetting->setIcon(QIcon(":/JZNodeEditor/Resources/icons/iconSetting.png"));        

        QVBoxLayout *l = new QVBoxLayout();
        l->addWidget(tool);

        table = new QTableWidget();
        l->addWidget(table);
        this->setLayout(l);
    }

    QTableWidget *table;
    QToolButton *btnStart, *btnStop, *btnSetting;
};


//
JZModbusSimulator::Simulator::Simulator()
{
    master = nullptr;
    slaver = nullptr;
    table = nullptr;
}

bool JZModbusSimulator::Simulator::isOpen()
{
    if (master && master->isOpen())
        return true;

    if (slaver && slaver->isStart())
        return true;

    return false;
}

void JZModbusSimulator::Simulator::close()
{
    if (master)
    {
        master->close();
        delete master;
        master = nullptr;
    }

    if (slaver)
    {
        slaver->stopServer();
        delete slaver;
        slaver = nullptr;
    }
}


//JZModbusSimulator
JZModbusSimulator::JZModbusSimulator(QWidget *parent)
    :QWidget(parent)
{
    this->setWindowFlag(Qt::Window);

    m_simIdx = 1;
    m_dataBitsList << QSerialPort::Data5 << QSerialPort::Data6 << QSerialPort::Data7 << QSerialPort::Data8;
    m_stopBitsList << QSerialPort::OneStop << QSerialPort::TwoStop;
    m_parityList << QSerialPort::NoParity << QSerialPort::EvenParity << QSerialPort::OddParity;      

    QWidget *widget = new QWidget();
    QVBoxLayout *center = new QVBoxLayout();
    center->setContentsMargins(9, 9, 9, 9);
    widget->setLayout(center);

    QWidget *widget_left = new QWidget();
    QVBoxLayout *l_left = new QVBoxLayout();
    l_left->setContentsMargins(0, 0, 0, 0);
    widget_left->setLayout(l_left);

    m_tree = new QTreeWidget();
    m_tree->setHeaderHidden(true);    
    m_tree->setExpandsOnDoubleClick(false);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree, &QWidget::customContextMenuRequested, this, &JZModbusSimulator::onContextMenu);

    //main
    QSplitter *splitterMain = new QSplitter(Qt::Horizontal);
    splitterMain->setObjectName("splitterMain");
    splitterMain->addWidget(m_tree);
    splitterMain->addWidget(widget_left);

    center->addWidget(splitterMain);

    //left
    m_mdiArea = new QMdiArea();    
    m_log = new QPlainTextEdit();

    QSplitter *splitterLeft = new QSplitter(Qt::Vertical);
    splitterLeft->addWidget(m_mdiArea);
    splitterLeft->addWidget(m_log);
    l_left->addWidget(splitterLeft);

    splitterMain->setCollapsible(0, false);
    splitterMain->setCollapsible(1, false);
    splitterMain->setStretchFactor(0, 0);
    splitterMain->setStretchFactor(1, 1);
    splitterMain->setSizes({ 250,600 });

    splitterLeft->setCollapsible(0, false);
    splitterLeft->setCollapsible(1, false);
    splitterLeft->setStretchFactor(0, 1);
    splitterLeft->setStretchFactor(1, 0);

    QVBoxLayout *main_layout = new QVBoxLayout();    
    main_layout->setContentsMargins(0, 0, 0, 0);

    QMenuBar *menubar = new QMenuBar();    
    main_layout->addWidget(menubar);
    main_layout->addWidget(widget);
    this->setLayout(main_layout);

    QMenu *menu_file = menubar->addMenu("文件");
    auto actNew = menu_file->addAction("新建设备");
    auto actClear = menu_file->addAction("清空设备");
    connect(actNew, &QAction::triggered, this, &JZModbusSimulator::onActionNew);
    connect(actClear, &QAction::triggered, this, &JZModbusSimulator::onActionClear);
    menu_file->addSeparator();

    auto actSaveConfig = menu_file->addAction("保存配置");
    auto actLoadConfig = menu_file->addAction("加载配置");
    connect(actSaveConfig, &QAction::triggered, this, &JZModbusSimulator::onActionSaveConfig);
    connect(actLoadConfig, &QAction::triggered, this, &JZModbusSimulator::onActionLoadConfig);

    auto menu_view = menubar->addMenu("视图");
    auto actShowAll = menu_view->addAction("显示全部");
    connect(actShowAll, &QAction::triggered, this, &JZModbusSimulator::onActionShowAll);

    this->adjustSize();
}

JZModbusSimulator::~JZModbusSimulator()
{
    closeAll();
}

void JZModbusSimulator::closeAll()
{
    int count = m_simulator.size();
    for (int i = 0; i < count; i++)
        removeSimulator(0);    
    m_simulator.clear();
    m_simIdx = 0;
}

bool JZModbusSimulator::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Close)
    {
        QMdiSubWindow *w = qobject_cast<QMdiSubWindow*>(o);
        w->showMinimized();
        e->ignore();
        return true;
    }    
    
    return QWidget::eventFilter(o, e);
}

void JZModbusSimulator::addSimulator(JZModbusConfig config)
{
    Simulator info;
    info.config = config;

    SimulatorWidget *simulator = new SimulatorWidget();
    info.widget = simulator;
    connect(simulator->btnStart, &QToolButton::clicked, this, &JZModbusSimulator::onSimulatorStart);
    connect(simulator->btnStop, &QToolButton::clicked, this, &JZModbusSimulator::onSimulatorStop);
    connect(simulator->btnSetting, &QToolButton::clicked, this, &JZModbusSimulator::onSimulatorSetting);

    auto table = simulator->table;
    QStringList headers = { "地址","功能","类型","值","操作","策略","备注" };
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    connect(table, &QTableWidget::itemChanged, this, &JZModbusSimulator::onItemChanged);

    simulator->btnStart->setProperty("table", QVariant::fromValue(table));
    simulator->btnStop->setProperty("table", QVariant::fromValue(table));
    simulator->btnSetting->setProperty("table", QVariant::fromValue(table));
    
    info.table = table;
    info.window = m_mdiArea->addSubWindow(simulator);
    info.window->show();
    info.window->installEventFilter(this);         

    QTreeWidgetItem *item = new QTreeWidgetItem();
    info.item = item;
    item->setText(0,"Device" + QString::number(m_simIdx++));
    info.window->setWindowTitle(item->text(0));
    m_tree->addTopLevelItem(item);

    m_simulator.push_back(info);
    initSimulator(m_simulator.size() - 1);    
}

void JZModbusSimulator::removeSimulator(int index)
{    
    m_simulator[index].close();
    m_mdiArea->removeSubWindow(m_simulator[index].window);    
    delete m_simulator[index].item;
    m_simulator.removeAt(index);
}

void JZModbusSimulator::startSimulator(int index)
{
    auto &info = m_simulator[index];
    auto &c = info.config;

    bool ret = false;
    int type = c.modbusType;
    if (type == Modbus_rtuClient || type == Modbus_tcpClient)
        ret = info.master->open();
    else if (type == Modbus_rtuServer || type == Modbus_tcpServer)
        ret = info.slaver->startServer();

    if (!ret)
    {
        QMessageBox::information(this, "", "启动失败,请检查设置");
        return;
    }
    updateStatus(index);
}

void JZModbusSimulator::stopSimulator(int index)
{
    auto &info = m_simulator[index];
    auto &c = info.config;

    bool ret = false;
    int type = c.modbusType;
    if (type == Modbus_rtuClient || type == Modbus_tcpClient)
        info.master->close();
    else if (type == Modbus_rtuServer || type == Modbus_tcpServer)
        info.slaver->stopServer();

    updateStatus(index);
}

void JZModbusSimulator::initSimulator(int index)
{
    auto &info = m_simulator[index];
    
    info.close();
    if (info.config.modbusType == Modbus_rtuClient || info.config.modbusType == Modbus_tcpClient)
    {
        info.master = new JZModbusMaster();
        modbusMasterSetConfig(info.master, &info.config);
        connect(info.master, &JZModbusMaster::sigParamChanged, this, &JZModbusSimulator::onParamChanged);
        info.master->setProperty("table", QVariant::fromValue(info.table));
    }
    else
    {
        info.table->hideColumn(4);
        info.table->hideColumn(5);
        info.slaver = new JZModbusSlaver();
        modbusSlaverSetConfig(info.slaver, &info.config);
        connect(info.slaver, &JZModbusSlaver::sigParamChanged, this, &JZModbusSimulator::onParamChanged);
        info.slaver->setProperty("table", QVariant::fromValue(info.table));
    }
    updateTable(index);
}

void JZModbusSimulator::settingSimulator(int index)
{
    auto &info = m_simulator[index];

    JZModbusConfigDialog dlg(this);
    dlg.setConfig(info.config);
    if (dlg.exec() != QDialog::Accepted)
        return;

    info.config = dlg.config();
    initSimulator(index);
}

void JZModbusSimulator::updateStatus(int index)
{
    auto &info = m_simulator[index];
    info.widget->btnStart->setEnabled(true);
    info.widget->btnStop->setEnabled(true);
    info.widget->btnSetting->setEnabled(true);
    if (info.isOpen())
    {
        info.widget->btnStart->setEnabled(false);
        info.widget->btnSetting->setEnabled(false);
    }
    else
    {
        info.widget->btnStop->setEnabled(false);
    }
}

void JZModbusSimulator::updateTable(int index)
{
    auto &info = m_simulator[index];
    QTableWidget *table = info.table;

    table->clearContents();
    table->blockSignals(true);

    JZModbusParamMap *map = nullptr;
    if (info.master)
        map = info.master->map();
    else
        map = info.slaver->map();
    
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
        itemAddr->setData(Qt::UserRole,proto->addr);        
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
        connect(btnRead, SIGNAL(clicked()), this, SLOT(onProtoReadClicked()));
        layout->addWidget(btnRead);

        QPushButton *btnWrite = new QPushButton("写入");
        btnWrite->setProperty("table", QVariant::fromValue(table));
        connect(btnWrite, SIGNAL(clicked()), this, SLOT(onProtoWriteClicked()));
        layout->addWidget(btnWrite);
        if (proto->addrType == Param_DiscreteInput || proto->addrType == Param_InputRegister)
            btnWrite->setEnabled(false);

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

void JZModbusSimulator::onItemChanged(QTableWidgetItem *item)
{
    auto table = item->tableWidget();
    int index = indexOfTable(table);
    auto &info = m_simulator[index];

    int row = item->row();
    int addr = table->item(row, 0)->data(Qt::UserRole).toInt();
    QVariant value = item->text();
    if (info.slaver)
        info.slaver->writeParam(addr, value);
    if (info.master)
        info.master->writeParam(addr, value);
}

int JZModbusSimulator::indexOfTable(QTableWidget *table)
{
    for (int i = 0; i < m_simulator.size(); i++)
    {        
        if (m_simulator[i].table == table)
            return i;
    }
    return -1;
}

int JZModbusSimulator::indexOfRow(QTableWidget *table,int addr)
{
    for (int i = 0; i < table->rowCount(); i++)
    {
        int item_addr = table->item(i, 0)->data(Qt::UserRole).toInt();
        if (item_addr == addr)
            return i;
    }
    return -1;
}

void JZModbusSimulator::onParamChanged(int addr)
{
    QObject *obj = sender();
    auto table = (QTableWidget*)obj->property("table").value<QObject*>();
    auto &info = m_simulator[indexOfTable(table)];

    QVariant v;
    if (info.master)
        v = info.master->readParam(addr);
    if (info.slaver)
        v = info.slaver->readParam(addr);

    int idx = indexOfRow(table,addr);
    table->item(idx, 3)->setText(v.toString());

    m_log->appendPlainText("收到数据");
}

void JZModbusSimulator::onProtoReadClicked()
{
    auto btn = qobject_cast<QPushButton*>(sender());
    auto table = (QTableWidget*)btn->property("table").value<QObject*>();    
    auto &info = m_simulator[indexOfTable(table)];
    if (info.master->isBusy())
        return;
    
    int addr = btn->property("addr").toInt();
    info.master->readRemoteParamAsync(addr);
}

void JZModbusSimulator::onProtoWriteClicked()
{
    auto btn = qobject_cast<QPushButton*>(sender());
    auto table = (QTableWidget*)btn->property("table").value<QObject*>();
    auto &info = m_simulator[indexOfTable(table)];
    if (info.master->isBusy())
        return;
    
    int addr = btn->property("addr").toInt();
    int row = indexOfRow(info.table,addr);
    
    QVariant v = table->item(row, 3)->text();        
    info.master->writeRemoteParamAsync(addr,v);
}

void JZModbusSimulator::onSimulatorStart()
{
    auto btn = qobject_cast<QToolButton*>(sender());
    auto table = (QTableWidget*)btn->property("table").value<QObject*>();
    startSimulator(indexOfTable(table));
}

void JZModbusSimulator::onSimulatorStop()
{
    auto btn = qobject_cast<QToolButton*>(sender());
    auto table = (QTableWidget*)btn->property("table").value<QObject*>();
    stopSimulator(indexOfTable(table));
}

void JZModbusSimulator::onSimulatorSetting()
{
    auto btn = qobject_cast<QToolButton*>(sender());
    auto table = (QTableWidget*)btn->property("table").value<QObject*>();
    settingSimulator(indexOfTable(table));    
}

void JZModbusSimulator::onProtoStrategyClicked()
{
    auto btn = qobject_cast<QPushButton*>(sender());
    auto table = (QTableWidget*)btn->property("table").value<QObject*>();
    auto &info = m_simulator[indexOfTable(table)];    
    int addr = btn->property("addr").toInt();

    ModeStargeDialog dlg(this);
    if(info.config.strategyMap.contains(addr))
        dlg.setInfo(info.config.strategyMap[addr]);
    if (dlg.exec() != QDialog::Accepted)
        return;

    info.config.strategyMap[addr] = dlg.info();
    info.master->setStrategy(addr, dlg.info());
}

void JZModbusSimulator::onActionNew()
{
    JZModbusConfigDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    addSimulator(dlg.config());
}

void JZModbusSimulator::onActionClear()
{
    closeAll();
}

void JZModbusSimulator::onActionShowAll()
{
    for (int i = 0; i < m_simulator.size(); i++)
        m_simulator[i].window->show();
}

void JZModbusSimulator::onContextMenu(QPoint pt)
{
    QMenu menu(this);
    auto item = m_tree->itemAt(pt);
    QAction *actDel = nullptr;
    if (!item)
    {
        auto actNew = menu.addAction("新建");
        connect(actNew, &QAction::triggered, this, &JZModbusSimulator::onActionNew);
    }
    else
    {
        actDel = menu.addAction("删除");
    }

    auto act = menu.exec(m_tree->mapToGlobal(pt));
    if (!act)
        return;

    if (act == actDel)
    {
        int idx = m_tree->indexOfTopLevelItem(item);
        removeSimulator(idx);
        delete m_tree->takeTopLevelItem(idx);
    }
}

void JZModbusSimulator::onActionSaveConfig()
{
    QString path = QFileDialog::getSaveFileName(this, "", "modbus.jzcfg");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return;
    
    QList<JZModbusConfig> cfg_list;
    for (int i = 0; i < m_simulator.size(); i++)
        cfg_list << m_simulator[i].config;

    QDataStream s(&file);    
    s << cfg_list;
    file.close();
}

void JZModbusSimulator::onActionLoadConfig()
{
    QString path = QFileDialog::getOpenFileName(this, "", "*.jzcfg");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QFile::ReadOnly))
        return;

    closeAll();    

    QList<JZModbusConfig> cfg_list;
    QDataStream s(&file);            
    s >> cfg_list;
    for (int i = 0; i < cfg_list.size(); i++)
        addSimulator(cfg_list[i]);

    file.close();
}