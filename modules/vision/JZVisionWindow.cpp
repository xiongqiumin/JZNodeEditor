#include <QSplitter>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include "JZVisionWindow.h"

//JZVisionWindowConfig
QDataStream& operator<<(QDataStream& s, const JZVisionWindowConfig& param)
{    
    return s;
}

QDataStream& operator>>(QDataStream& s, JZVisionWindowConfig& param)
{
    return s;
}

//JZVisionWindow
JZVisionWindow::JZVisionWindow()
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

    splitterTop->setChildrenCollapsible(false);
    splitterMain->setChildrenCollapsible(false);

    setCentralWidget(splitterMain);
    resize(800, 600);
}

JZVisionWindow::~JZVisionWindow()
{
}

void JZVisionWindow::saveConfig()
{
}

JZVisionWindowConfig JZVisionWindow::config()
{
    return m_config;
}

JZModelManager* JZVisionWindow::modelManager()
{
    return m_modelManager;
}

JZCameraManager* JZVisionWindow::cameraManager()
{
    return m_cameraManager;
}

JZCommManager* JZVisionWindow::commManager()
{
    return m_commManager;
}

JZCameraListWidget* JZVisionWindow::cameraList()
{
    return m_list;
}

JZCameraViewWidget* JZVisionWindow::cameraView()
{
    return m_view;
}

void JZVisionWindow::init(JZVisionWindowConfig config)
{
    m_config = config;
}

void JZVisionWindow::initView()
{   
    initMenu();
    initToolBar();
}

void JZVisionWindow::initMenu()
{
    QMenuBar* bar = menuBar();
    bar->clear();

    auto menu_file = bar->addMenu("File");
    QAction *act_close = menu_file->addAction("close");
    connect(act_close,&QAction::triggered,this, &JZVisionWindow::onActionClose);

    auto menu_setting = bar->addMenu("Setting");

    QAction* act_camera = menu_setting->addAction("camera");
    QAction* act_model = menu_setting->addAction("model");
    QAction* act_comm = menu_setting->addAction("comm");
    QAction* act_database = menu_setting->addAction("db");
    QAction* act_mes = menu_setting->addAction("mes");
    connect(act_camera, &QAction::triggered, this, &JZVisionWindow::onActionCameraSetting);
    connect(act_model, &QAction::triggered, this, &JZVisionWindow::onActionModelSetting);
    connect(act_comm, &QAction::triggered, this, &JZVisionWindow::onActionCommSetting);
    connect(act_database, &QAction::triggered, this, &JZVisionWindow::onActionDatabaseSetting);
    connect(act_mes, &QAction::triggered, this, &JZVisionWindow::onActionMesSetting);

    if (m_cameraManager->cameraList().size() == 0)
        act_camera->setVisible(false);
    if (m_commManager->commList().size() == 0)
        act_comm->setVisible(false);
    if (m_modelManager->modelList().size() == 0)
        act_model->setVisible(false);

    act_database->setVisible(false);
    act_mes->setVisible(false);

    auto menu_help = bar->addMenu("Help");
    QAction* act_about = menu_help->addAction("about");
    connect(act_about, &QAction::triggered, this, &JZVisionWindow::onActionAbout);
}

void JZVisionWindow::initToolBar()
{
    auto tool_bar = addToolBar("tool");
    tool_bar->setMovable(false);

    QAction* act_once = tool_bar->addAction("StartOnce");
    QAction* act_start = tool_bar->addAction("Start");
    QAction* act_stop = tool_bar->addAction("Stop");
    connect(act_once, &QAction::triggered, this, &JZVisionWindow::onActionStartOnce);
    connect(act_start, &QAction::triggered, this, &JZVisionWindow::onActionStart);
    connect(act_stop, &QAction::triggered, this, &JZVisionWindow::onActionStop);
}

void JZVisionWindow::onActionClose()
{
    close();
}

bool JZVisionWindow::checkOpen(JZCamera *camera)
{
    if (camera->isOpen())
        return true;

    return camera->open();
}

void JZVisionWindow::onActionStartOnce()
{
    auto cam_list = m_cameraManager->cameraList();
    for (int i = 0; i < cam_list.size(); i++)
    {
        JZCamera *camera = cam_list[i];
        if (!checkOpen(camera))
            return;

        camera->startOnce();
    }
}

void JZVisionWindow::onActionStart()
{
    auto cam_list = m_cameraManager->cameraList();
    for (int i = 0; i < cam_list.size(); i++)
    {
        JZCamera* camera = cam_list[i];
        if (!checkOpen(camera))
            return;

        camera->start();
    }
}

void JZVisionWindow::onActionStop()
{
    auto cam_list = m_cameraManager->cameraList();
    for (int i = 0; i < cam_list.size(); i++)
    {
        JZCamera* camera = cam_list[i];
        camera->stop();
    }
}

void JZVisionWindow::onActionCameraSetting()
{
}

void JZVisionWindow::onActionModelSetting()
{
}

void JZVisionWindow::onActionCommSetting()
{
}

void JZVisionWindow::onActionDatabaseSetting()
{
}

void JZVisionWindow::onActionMesSetting()
{
}

void JZVisionWindow::onActionAbout()
{
    QMessageBox::information(this,"","hello world");
}