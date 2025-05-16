#include <QSplitter>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include "JZVisonWindow.h"

//JZVisonWindowConfig
QDataStream& operator<<(QDataStream& s, const JZVisonWindowConfig& param)
{
    return s;
}

QDataStream& operator>>(QDataStream& s, JZVisonWindowConfig& param)
{
    return s;
}

//JZVisonWindow
JZVisonWindow::JZVisonWindow()
{
    m_modelManager = new JZModelManager(this);
    m_cameraManager = new JZCameraManager(this);
    m_commManager = new JZCommManager(this);

	m_list = new JZCameraListWidget();
	m_view = new JZCameraViewWidget();
    m_log = new JZLogWidget();

    m_list->setCameraManager(m_cameraManager);
    m_list->setViewWidget(m_view);

    //main
    QSplitter* splitterTop = new QSplitter(Qt::Horizontal);
    splitterTop->addWidget(m_list);
    splitterTop->addWidget(m_view);
    splitterTop->setSizes({ 200,600 });

    QSplitter* splitterMain = new QSplitter(Qt::Vertical);
    splitterMain->addWidget(splitterTop);
    splitterMain->addWidget(m_log);
    splitterMain->setSizes({ 450,150 });

    setCentralWidget(splitterMain);
    resize(800, 600);
}

JZVisonWindow::~JZVisonWindow()
{
}

void JZVisonWindow::saveConfig()
{
}

JZVisonWindowConfig JZVisonWindow::config()
{
    return m_config;
}

void JZVisonWindow::init(JZVisonWindowConfig config)
{
    m_config = config;
    m_cameraManager->setConfig(config.cameraConfig);
    m_commManager->setConfig(config.commConfig);
    m_modelManager->setConfig(config.modelConfig);

    initMenu();
    initToolBar();
}

void JZVisonWindow::initMenu()
{
    QMenuBar* bar = new QMenuBar();
    setMenuBar(bar);

    auto menu_file = bar->addMenu("File");
    QAction *act_close = menu_file->addAction("close");
    connect(act_close,&QAction::triggered,this, &JZVisonWindow::onActionClose);

    auto menu_setting = bar->addMenu("Setting");

    QAction* act_camera = menu_file->addAction("camera");
    QAction* act_model = menu_file->addAction("model");
    QAction* act_comm = menu_file->addAction("comm");
    QAction* act_database = menu_file->addAction("db");
    QAction* act_mes = menu_file->addAction("mes");
    connect(act_camera, &QAction::triggered, this, &JZVisonWindow::onActionCameraSetting);
    connect(act_model, &QAction::triggered, this, &JZVisonWindow::onActionModelSetting);
    connect(act_comm, &QAction::triggered, this, &JZVisonWindow::onActionCommSetting);
    connect(act_database, &QAction::triggered, this, &JZVisonWindow::onActionDatabaseSetting);
    connect(act_mes, &QAction::triggered, this, &JZVisonWindow::onActionMesSetting);

    if (m_cameraManager->cameraList().size() == 0)
        act_camera->setVisible(false);
    if (m_config.commConfig.commList.size() == 0)
        act_comm->setVisible(false);
    if (m_config.modelConfig.modelList.size() == 0)
        act_model->setVisible(false);

    act_database->setVisible(false);
    act_mes->setVisible(false);

    auto menu_help = bar->addMenu("Help");
    QAction* act_about = menu_help->addAction("about");
    connect(act_about, &QAction::triggered, this, &JZVisonWindow::onActionAbout);
}

void JZVisonWindow::initToolBar()
{
    auto tool_bar = addToolBar("tool");
    tool_bar->setMovable(false);

    QAction* act_once = tool_bar->addAction("StartOnce");
    QAction* act_start = tool_bar->addAction("Start");
    QAction* act_stop = tool_bar->addAction("Stop");
    connect(act_once, &QAction::triggered, this, &JZVisonWindow::onActionStartOnce);
    connect(act_start, &QAction::triggered, this, &JZVisonWindow::onActionStart);
    connect(act_stop, &QAction::triggered, this, &JZVisonWindow::onActionStop);
}

void JZVisonWindow::onActionClose()
{
    close();
}

void JZVisonWindow::onActionStartOnce()
{
    auto cam_list = m_cameraManager->cameraList();
    for (int i = 0; i < cam_list.size(); i++)
    {
        JZCamera *camera = m_cameraManager->camera(cam_list[i]);
        camera->startOnce();
    }
}

void JZVisonWindow::onActionStart()
{
    auto cam_list = m_cameraManager->cameraList();
    for (int i = 0; i < cam_list.size(); i++)
    {
        JZCamera* camera = m_cameraManager->camera(cam_list[i]);
        camera->start();
    }
}

void JZVisonWindow::onActionStop()
{
    auto cam_list = m_cameraManager->cameraList();
    for (int i = 0; i < cam_list.size(); i++)
    {
        JZCamera* camera = m_cameraManager->camera(cam_list[i]);
        camera->stop();
    }
}

void JZVisonWindow::onActionCameraSetting()
{
}

void JZVisonWindow::onActionModelSetting()
{
}

void JZVisonWindow::onActionCommSetting()
{
}

void JZVisonWindow::onActionDatabaseSetting()
{
}

void JZVisonWindow::onActionMesSetting()
{
}

void JZVisonWindow::onActionAbout()
{
    QMessageBox::information(this,"","hello world");
}