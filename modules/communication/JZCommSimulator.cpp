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
#include <QMenuBar>
#include "JZCommSimulator.h"
#include "JZRegExpHelp.h"
#include "modbus/JZModbusSimulator.h"
#include "JZNodeUtils.h"

JZCommSimulatorWidgetConfig::JZCommSimulatorWidgetConfig()
{
    type = Sim_None;
}

void JZCommSimulatorWidgetConfig::init(JZCommSimulatorType t)
{
    type = t;
    if (t == Sim_Modbus)
    {
        name = "modbus";

        JZModbusConfig config;
        buffer = JZNodeUtils::toBuffer(config);
    }
    else if (t == Sim_Net)
    {
        name = "net";
    }
    else if (t == Sim_SerialPort)
    {
        name = "serial";
    }
}

//JZCommSimulatorConfig
QDataStream &operator<<(QDataStream &s, const JZCommSimulatorWidgetConfig &param)
{
    s << param.type << param.buffer;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZCommSimulatorWidgetConfig &param)
{
    s >> param.type >> param.buffer;
    return s;
}

QDataStream &operator<<(QDataStream &s, const JZCommSimulatorConfig &param)
{
    s << param.commList;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZCommSimulatorConfig &param)
{
    s >> param.commList;
    return s;
}

//JZCommSimulator
JZCommSimulator::JZCommSimulator(QWidget *parent)
    :QWidget(parent)
{
    this->setWindowFlag(Qt::Window);
    this->setAttribute(Qt::WA_DeleteOnClose);
            
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
    connect(m_tree, &QWidget::customContextMenuRequested, this, &JZCommSimulator::onContextMenu);
    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &JZCommSimulator::onItemDoubleClicked);

    //main
    QSplitter *splitterMain = new QSplitter(Qt::Horizontal);
    splitterMain->setObjectName("splitterMain");
    splitterMain->addWidget(m_tree);
    splitterMain->addWidget(widget_left);

    center->addWidget(splitterMain);

    //left
    m_mdiArea = new QMdiArea();    
    m_log = new JZLogWidget();

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
    
    
    //main menu
    QMenuBar *menubar = new QMenuBar();        

    QMenu *menu_file = menubar->addMenu("文件");
    auto actNew = menu_file->addAction("新建设备");
    auto actClear = menu_file->addAction("清空设备");
    connect(actNew, &QAction::triggered, this, &JZCommSimulator::onActionNew);
    connect(actClear, &QAction::triggered, this, &JZCommSimulator::onActionClear);
    menu_file->addSeparator();

    auto actSaveConfig = menu_file->addAction("保存配置");
    auto actLoadConfig = menu_file->addAction("加载配置");
    connect(actSaveConfig, &QAction::triggered, this, &JZCommSimulator::onActionSaveConfig);
    connect(actLoadConfig, &QAction::triggered, this, &JZCommSimulator::onActionLoadConfig);

    auto menu_view = menubar->addMenu("视图");
    auto actShowAll = menu_view->addAction("显示全部");
    connect(actShowAll, &QAction::triggered, this, &JZCommSimulator::onActionShowAll);

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(menubar);
    l->addWidget(widget);
    setLayout(l);

    this->adjustSize();
}

JZCommSimulator::~JZCommSimulator()
{
    closeAll();
}

void JZCommSimulator::closeAll()
{
    int count = m_simulator.size();
    for (int i = 0; i < count; i++)
        removeSimulator(0);    
    m_simulator.clear();    
}


void JZCommSimulator::setConfig(JZCommSimulatorConfig config)
{
    closeAll();

    for (int i = 0; i < config.commList.size(); i++)
        addSimulator(config.commList[i]);
}

JZCommSimulatorConfig JZCommSimulator::config()
{
    JZCommSimulatorConfig config;
    for (int i = 0; i < m_simulator.size(); i++)
    {
        JZCommSimulatorWidgetConfig widget_config;
        widget_config.name = m_simulator[i].item->text(0);
        widget_config.type = m_simulator[i].widget->type();
        widget_config.buffer = m_simulator[i].widget->getConfig();
        config.commList << widget_config;
    }

    return config;
}

void JZCommSimulator::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    emit sigClose();
}

bool JZCommSimulator::eventFilter(QObject *o, QEvent *e)
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

void JZCommSimulator::addSimulator(JZCommSimulatorWidgetConfig config)
{
    Simulator info;
    
    JZCommSimulatorWidget* widget = nullptr;
    if (config.type == Sim_Modbus)
    {
        widget = new JZModBusSimulator();
    }
    else if (config.type == Sim_Net)
    {

    }
    else if (config.type == Sim_SerialPort)
    {

    }

    info.widget = widget;

    info.window = m_mdiArea->addSubWindow(widget);
    info.window->show();
    info.window->installEventFilter(this);         

    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, config.name);
    info.item = item;    
    m_tree->addTopLevelItem(item);
    
    m_simulator.push_back(info);   
}

void JZCommSimulator::removeSimulator(int index)
{    
    m_simulator[index].widget->close();
    m_mdiArea->removeSubWindow(m_simulator[index].window);    
    delete m_simulator[index].item;
    m_simulator.removeAt(index);
}

void JZCommSimulator::onActionNew()
{
    auto type = (JZCommSimulatorType)sender()->property("SimType").toInt();

    JZCommSimulatorWidgetConfig config;
    config.init(type);

    QStringList nameList;
    for (int i = 0; i < m_simulator.size(); i++)
        nameList << m_simulator[i].item->text(0);

    config.name = JZRegExpHelp::uniqueString(config.name, nameList);
    addSimulator(config);
}

void JZCommSimulator::onActionClear()
{
    closeAll();
}

void JZCommSimulator::onActionShowAll()
{
    for (int i = 0; i < m_simulator.size(); i++)
        m_simulator[i].window->show();
}

void JZCommSimulator::onItemDoubleClicked(QTreeWidgetItem *item)
{
    int idx = m_tree->indexOfTopLevelItem(item);
    m_simulator[idx].window->raise();
    m_simulator[idx].window->activateWindow();
}

void JZCommSimulator::onContextMenu(QPoint pt)
{
    QMenu menu(this);
    auto item = m_tree->itemAt(pt);
    QAction *actDel = nullptr;
    if (!item)
    {
        auto actMenu = menu.addMenu("新建");
        auto actModbus = actMenu->addAction("Modbus");
        auto actNet = actMenu->addAction("Net");
        auto actSerial = actMenu->addAction("Serial");
        actModbus->setProperty("SimType", (int)Sim_Modbus);
        actNet->setProperty("SimType", (int)Sim_Net);
        actSerial->setProperty("SimType", (int)Sim_SerialPort);

        connect(actModbus, &QAction::triggered, this, &JZCommSimulator::onActionNew);
        connect(actNet, &QAction::triggered, this, &JZCommSimulator::onActionNew);
        connect(actSerial, &QAction::triggered, this, &JZCommSimulator::onActionNew);
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

void JZCommSimulator::onActionSaveConfig()
{
    QString path = QFileDialog::getSaveFileName(this, "", "modbus.jzcfg");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Truncate))
        return;
    
    auto cfg = config();

    QDataStream s(&file);    
    s << cfg;
    file.close();
}

void JZCommSimulator::onActionLoadConfig()
{
    QString path = QFileDialog::getOpenFileName(this, "", "*.jzcfg");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QFile::ReadOnly))
        return;

    closeAll();    
    JZCommSimulatorConfig cfg;

    QList<JZModbusConfig> cfg_list;
    QDataStream s(&file);            
    s >> cfg;
    file.close();

    setConfig(cfg);
}