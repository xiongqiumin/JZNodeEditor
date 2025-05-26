#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QFrame>
#include <QMenuBar>
#include "mainwindow.h"
#include "JZTitleWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{            
    setMinimumSize(800, 600);

    m_modelManager = new JZModelManager(this);
    m_cameraManager = new JZCameraManager(this);
    m_commManager = new JZCommManager(this);

    m_list = new JZCameraListWidget();
    m_view = new JZCameraViewWidget();
    m_flow = new JZFlowTreeWidget();
    
    m_list->setCameraManager(m_cameraManager);
    m_list->setViewWidget(m_view);    

    initUi();
}

MainWindow::~MainWindow()
{
}

QIcon MainWindow::icon(QString name)
{
    QString icon_path = ":/JZMonitorSystem/Resources/icons/" + name;
    return QIcon(icon_path);
}

void MainWindow::initUi()
{
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);        
    
    //title
    auto title = createTitleBar();
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    centralWidget->setLayout(mainLayout);
    mainLayout->addWidget(title);
    mainLayout->setSpacing(1);

    QMenuBar *bar = createMenuBar();
    mainLayout->addWidget(bar);

    //bottom
    QHBoxLayout *bottom_layout = new QHBoxLayout();        
    bottom_layout->setContentsMargins(6, 6, 6, 6);
    bottom_layout->setSpacing(0);

    QWidget *buttonWidget = new QWidget();    
    bottom_layout->addWidget(buttonWidget);
    buttonWidget->setStyleSheet("background-color: rgb(207,214,229);");

    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(3, 9, 3, 9);

    // 创建并添加按钮    
    QToolButton *button_camera = new QToolButton();
    QToolButton *button_flow = new QToolButton();
    QToolButton *button_model = new QToolButton();
    QToolButton *button_comm = new QToolButton();
    QToolButton *button_setting = new QToolButton();
    
    button_camera->setText("相机");
    button_camera->setIcon(icon("camera.png"));
    button_flow->setText("流程");
    button_flow->setIcon(icon("flow.png"));
    button_model->setText("模型");
    button_model->setIcon(icon("model.png"));
    button_comm->setText("通信");
    button_comm->setIcon(icon("comm.png"));
    button_setting->setText("设置");    
    button_setting->setIcon(icon("setting.png"));    
    
    connect(button_camera,&QToolButton::clicked,this, &MainWindow::onBtnCamera);
    connect(button_flow, &QToolButton::clicked, this, &MainWindow::onBtnFlow);

    QList<QToolButton*> btn_list;
    btn_list << button_camera;
    btn_list << button_flow;
    btn_list << button_model;
    btn_list << button_comm;
    btn_list << button_setting;
    for (int i = 0; i < btn_list.size(); i++)
    {
        buttonLayout->addWidget(btn_list[i]);        
        btn_list[i]->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn_list[i]->setIconSize(QSize(32, 32));        
    }
    buttonLayout->addStretch(1);                                    

    QWidget *line = new QWidget();
    line->setFixedWidth(3);
    line->setStyleSheet("background-color: rgb(41,57,85);");
    bottom_layout->addWidget(line);

    m_stack = new QStackedWidget();
    bottom_layout->addWidget(m_stack);

    addCameraPage();
    addFlowPage();
    addModelPage();
    addCommPage();
    addSettingPage();

    QWidget *bottom_widget = new QWidget();
    bottom_widget->setLayout(bottom_layout);
    bottom_widget->setObjectName("BottomWidget");
    bottom_widget->setStyleSheet("#BottomWidget { border: 6px solid rgb(41,57,85); } ");
    mainLayout->addWidget(bottom_widget);
}

QMenuBar *MainWindow::createMenuBar()
{
    QMenuBar *menubar = new QMenuBar();

    QMenu *menu_file = menubar->addMenu("文件");
    auto actNewMenu = menu_file->addAction("新建工程");
    auto actOpenMenu = menu_file->addAction("打开工程");
    connect(actNewMenu, &QAction::triggered, this, &MainWindow::onActionNewProject);
    connect(actOpenMenu, &QAction::triggered, this, &MainWindow::onActionOpenProject);

    QMenu *menu_help = menubar->addMenu("帮助");
    auto actHelp = menu_help->addAction("查看帮助");
    menu_help->addSeparator();    
    auto actAbout = menu_help->addAction("关于" + windowTitle());    
    connect(actHelp, &QAction::triggered, this, &MainWindow::onActionHelp);

    return menubar;
}

QWidget *MainWindow::createTitleBar()
{
    QHBoxLayout *l = new QHBoxLayout();

    QWidget *title_bar = new QWidget();
    title_bar->setLayout(l);

    JZTitleWidget *title = new JZTitleWidget();
    title->setFixedWidth(300);
    title->setFixedHeight(50);
    l->addWidget(title);            

    l->addStretch();

    title_bar->setStyleSheet("background-color: rgb(214,219,233);");

    return title_bar;
}

void MainWindow::onBtnCamera()
{
    m_stack->setCurrentIndex(0);
}

void MainWindow::onBtnFlow()
{
    m_stack->setCurrentIndex(1);
}

void MainWindow::onBtnModel()
{
    m_stack->setCurrentIndex(2);
}

void MainWindow::onBtnComm()
{
    m_stack->setCurrentIndex(3);
}

void MainWindow::onBtnSetting()
{
    m_stack->setCurrentIndex(4);
}

void MainWindow::onActionNewProject()
{

}

void MainWindow::onActionOpenProject()
{


}

void MainWindow::onActionHelp()
{

}

void MainWindow::setSliderStyle(QWidget *w)
{
    w->setStyleSheet(
        R"(QSplitter::handle
        {
            background-color: rgb(41,57,85);
            width: 6px;
            height: 6px;
        })"
    );
}

void MainWindow::addCameraPage()
{            
    JZPanelWidget *panel = new JZPanelWidget();
    panel->addTab("设备列表", m_list);

    QSplitter* splitterTop = new QSplitter(Qt::Horizontal);
    splitterTop->addWidget(panel);
    splitterTop->addWidget(m_view);
    splitterTop->setSizes({ 200,600 });
    splitterTop->setStretchFactor(0, 0);
    splitterTop->setStretchFactor(1, 1);

    splitterTop->setChildrenCollapsible(false);
    setSliderStyle(splitterTop);
    m_stack->addWidget(splitterTop);
}

void MainWindow::addFlowPage()
{    
    JZPanelWidget *panel = new JZPanelWidget();
    panel->addTab("流程列表",m_flow);    

    QSplitter* splitterTop = new QSplitter(Qt::Horizontal);    
    splitterTop->addWidget(panel);
    splitterTop->addWidget(new QWidget());
    splitterTop->setSizes({ 200,600 });
    splitterTop->setStretchFactor(0, 0);
    splitterTop->setStretchFactor(1, 1);

    splitterTop->setChildrenCollapsible(false);
    setSliderStyle(splitterTop);
    m_stack->addWidget(splitterTop);
}

void MainWindow::addModelPage()
{
    QWidget *w = new QWidget();
    m_stack->addWidget(w);
}

void MainWindow::addCommPage()
{
    QWidget *w = new QWidget();
    m_stack->addWidget(w);
}

void MainWindow::addSettingPage()
{
    QWidget *w = new QWidget();
    m_stack->addWidget(w);
}