#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QFrame>
#include <QMenuBar>
#include <QToolBar>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QApplication>
#include <QDesktopServices>
#include "mainwindow.h"
#include "JZTitleWidget.h"
#include "JZEditorUtils.h"
#include "JZNewProjectDialog.h"
#include "jzWidgets/JZLogWidget.h"
#include "modules/camera/JZCameraNode.h"
#include "modules/model/JZModelNode.h"
#include "modules/communication/JZCommNode.h"
#include "modules/communication/JZCommSimulator.h"
#include "JZModuleVisionApp.h"
#include "JZProjectTemplate.h"
#include "LogManager.h"
#include "JZEventWidget.h"
#include "JZEditorGlobal.h"
#include "JZDockWidget.h"
#include "widgets/JZAboutDialog.h"
#include "modules/opencv/CvToQt.h"
#include "JZVisionAppSample.h"
#include "JZEngineCoroutine.h"
#include "JZVisionUtils.h"

//Setting
Setting::Setting()
{

}

void Setting::addRecentProject(QString file)
{
    recentFile.removeAll(file);
    recentFile.insert(0, file);
    if (recentFile.size() > 10)
        recentFile.pop_back();
}

QDataStream& operator<<(QDataStream& s, const Setting& param)
{
    s << param.recentFile;
    return s;
}

QDataStream& operator >> (QDataStream& s, Setting& param)
{
    s >> param.recentFile;
    return s;
}

//MainWindow
MainWindow *g_visionWindow = nullptr;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{            
    g_visionWindow = this;
    m_traceView = nullptr;
    m_running = false;

    m_className = "MyVisionApp";
    setMinimumSize(800, 600);
    setEditorProject(&m_project);
    setWindowTitle("JZVison");

    m_roiFunction.insert("QList<JZYoloResult>", "JZYoloResult::toGraphics");
    m_roiFunction.insert("QList<JZOCRResult>", "JZOCRResult::toGraphics");
    m_roiFunction.insert("QList<JZQRCodeResult>", "JZQRCodeResult::toGraphics");
    m_roiFunction.insert("QList<JZBarCodeResult>", "JZBarCodeResult::toGraphics");    

    m_task.setProject(&m_project);
    connect(m_task.runThread(), &JZNodeAutoRunThread::sigResult, this, &MainWindow::onAutoRunResult);
    connect(&m_task, &MainTaskManager::sigBuildStart, this, &MainWindow::onBuildStart);
    connect(&m_task, &MainTaskManager::sigBuildFinish, this, &MainWindow::onBuildFinish);

    auto builder = m_task.buildThread()->buidler();
    connect(builder, &JZNodeBuilder::sigLog, this, &MainWindow::onBuildLog);

    connect(&m_engine, &JZNodeEngine::sigRuntimeError, this, &MainWindow::onRuntimeError, Qt::QueuedConnection);
    connect(&m_engine, &JZNodeEngine::sigNodeTrace, this, &MainWindow::onNodeTrace);
    connect(&m_engine, &JZNodeEngine::sigStatusChanged, this, &MainWindow::onEngineStatusChanged);
    
    JZEditorManager::instance()->registEditor(ProjectItem_scriptItem, CreateEditor<JZVisionEditor>);

    LogManagerInit();
    JZLogManager::instance()->addObserver(LogModule_General, this);
    JZLogManager::instance()->addObserver(Log_Compiler, this);
    JZLogManager::instance()->addObserver(Log_Runtime, this);

    m_modelManager = new JZModelManager(this);
    m_cameraManager = new JZCameraManager(this);
    m_commManager = new JZCommManager(this);
    connect(m_cameraManager, &JZCameraManager::sigFrameReady, this, &MainWindow::onFrameReady);
    connect(m_cameraManager, &JZCameraManager::sigError, this, &MainWindow::onCameraError);

    JZQRCode* m_qrCode = new JZQRCode();
    JZBarCode* m_barCode = new JZBarCode();
    JZPaddleOCR* m_ocr = new JZPaddleOCR();

    m_cameraList = new JZCameraListWidget();
    m_cameraView = new JZCameraViewWidget();
    m_projectTree = nullptr;
    m_editor = nullptr;

    connect(m_cameraList, &JZCameraListWidget::sigCameraChanged, this, &MainWindow::onCameraConfigChanged);    
    
    m_cameraList->setMainWindow(this);
    m_cameraList->setViewWidget(m_cameraView);    

    initUi();
    initDatabase();
    loadSetting();
}

MainWindow::~MainWindow()
{
    saveSetting();
}

JZProject* MainWindow::project()
{
    return &m_project;
}

void MainWindow::initDatabase()
{
    m_db.open(qApp->applicationDirPath() + "/config.db");
    if (!m_db.hasTable(m_dbConfig.tableName()))
    {
        m_db.createTable(m_dbConfig.table());
    }
}

void MainWindow::loadSetting()
{
    if(m_dbConfig.hasConfig("setting"))
        m_setting = m_dbConfig.getConfig<Setting>("setting");
    
    if (!m_setting.recentFile.isEmpty())
        openProject(m_setting.recentFile[0]);
    else
    {
        QString default_dir = qApp->applicationDirPath() + "/project/default";
        if (!QDir().exists(default_dir))
            QDir().mkpath(default_dir);

        QString project_path = default_dir + "/default.proj";
        if (!QFile::exists(project_path))
        {
            initProject();
            if (!m_project.saveAs(project_path) || !m_project.saveAllItem())
            {
                QMessageBox::information(this, "", "新建工程失败");
                return;
            }
        }
        openProject(project_path);        
    }
}

void MainWindow::saveSetting()
{
    m_dbConfig.setConfig<Setting>("setting", m_setting);
}

void MainWindow::setSliderStyle(QWidget* w)
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

JZModelManager* MainWindow::modelManager()
{
    return m_modelManager;
}

JZCameraManager* MainWindow::cameraManager()
{
    return m_cameraManager;
}

JZCommManager* MainWindow::commManager()
{
    return m_commManager;
}

JZPaddleOCR* MainWindow::getOCR()
{
    if (m_ocr->isInit())
    {
        m_ocr->init();
    }

    return m_ocr;
}

JZBarCode* MainWindow::getBarCode()
{
    if (m_barCode->isInit())
    {
        m_barCode->init();
    }

    return m_barCode;
}

JZQRCode* MainWindow::getQrCode()
{
    if (m_qrCode->isInit())
    {
        m_qrCode->init();
    }

    return m_qrCode;
}

void MainWindow::customEvent(QEvent* event)
{
    if (event->type() == JZLogEvent::EventType)
    {
        auto log_event = dynamic_cast<JZLogEvent*>(event);
        auto log = log_event->log;
        m_mainLog->addLog(log);
    }
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!closeProject())
    {
        event->ignore();
        return;
    }

    m_task.clearTask();
    m_task.stopRunThread();
    for (int i = 0; i < m_floatWindow.size(); i++)
        m_floatWindow[i]->close();
    releaseEngine();
    QMainWindow::closeEvent(event);
}

QIcon MainWindow::menuIcon(const QString &name)
{
    return QIcon(":/JZNodeEditor/Resources/icons/" + name);
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

    initMenuBar(mainLayout);        

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
    QToolButton* button_log = new QToolButton();
    QToolButton *button_setting = new QToolButton();    
    
    button_camera->setText("相机");
    button_camera->setIcon(JZVisionUtils::icon("camera.png"));
    button_flow->setText("流程");
    button_flow->setIcon(JZVisionUtils::icon("flow.png"));
    button_model->setText("模型");
    button_model->setIcon(JZVisionUtils::icon("model.png"));
    button_comm->setText("通信");
    button_comm->setIcon(JZVisionUtils::icon("comm.png"));
    button_log->setText("事件");
    button_log->setIcon(JZVisionUtils::icon("log.png"));
    button_setting->setText("设置");
    button_setting->setIcon(JZVisionUtils::icon("setting.png"));

    connect(button_camera,&QToolButton::clicked,this, &MainWindow::onBtnCamera);
    connect(button_flow, &QToolButton::clicked, this, &MainWindow::onBtnFlow);
    connect(button_model, &QToolButton::clicked, this, &MainWindow::onBtnModel);
    connect(button_comm, &QToolButton::clicked, this, &MainWindow::onBtnComm);
    connect(button_log, &QToolButton::clicked, this, &MainWindow::onBtnLog);
    connect(button_setting, &QToolButton::clicked, this, &MainWindow::onBtnSetting);    

    QList<QToolButton*> btn_list;
    btn_list << button_camera;
    btn_list << button_flow;
    btn_list << button_model;
    btn_list << button_comm;
    btn_list << button_log;
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
    connect(m_stack, &QStackedWidget::currentChanged, this, &MainWindow::onMainStackedChanged);

    addCameraPage();
    addFlowPage();
    addModelPage();
    addCommPage();
    addLogPage();
    addSettingPage();

    QWidget *bottom_widget = new QWidget();
    bottom_widget->setLayout(bottom_layout);
    bottom_widget->setObjectName("BottomWidget");
    bottom_widget->setStyleSheet("#BottomWidget { border: 6px solid rgb(41,57,85); } ");
    mainLayout->addWidget(bottom_widget);
}

void MainWindow::initMenuBar(QVBoxLayout *layout)
{
    QMenuBar *menubar = new QMenuBar();

    QMenu *menu_file = menubar->addMenu("文件");
    menu_file->setProperty("JZMenuType", Menu_File);
    auto actNewMenu = menu_file->addAction("新建工程");
    auto actOpenMenu = menu_file->addAction("打开工程");
    connect(actNewMenu, &QAction::triggered, this, &MainWindow::onActionNewProject);
    connect(actOpenMenu, &QAction::triggered, this, &MainWindow::onActionOpenProject);
    
    menu_file->addSeparator();
    auto actSaveFile = menu_file->addAction(menuIcon("iconSave.png"), "保存文件");
    auto actSaveAllFile = menu_file->addAction(menuIcon("iconSaveAll.png"), "全部保存");
    auto actCloseAllFile = menu_file->addAction("全部关闭");
    connect(actSaveFile, &QAction::triggered, this, &MainWindow::onActionSaveFile);
    connect(actSaveAllFile, &QAction::triggered, this, &MainWindow::onActionSaveAllFile);
    connect(actCloseAllFile, &QAction::triggered, this, &MainWindow::onActionCloseAllFile);

    menu_file->addSeparator();
    auto recent = menu_file->addMenu("最近使用过的项目");
    for (int i = 0; i < m_setting.recentFile.size(); i++) 
    {
        auto tmp = recent->addAction(m_setting.recentFile[i]);
        connect(tmp, &QAction::triggered, this, &MainWindow::onActionRecentProject);
    }  

    menu_file->addSeparator();
    auto actExit = menu_file->addAction("退出");
    connect(actExit, &QAction::triggered, this, &MainWindow::close);

    QMenu *menu_edit = menubar->addMenu("编辑");
    auto actUndo = menu_edit->addAction(menuIcon("iconUndo.png"), "撤销");
    auto actRedo = menu_edit->addAction(menuIcon("iconRedo.png"), "重做");
    menu_edit->addSeparator();
    auto actDel = menu_edit->addAction(menuIcon("iconDelete.png"), "删除");
    auto actCut = menu_edit->addAction(menuIcon("iconCut.png"), "剪切");
    auto actCopy = menu_edit->addAction(menuIcon("iconCopy.png"), "复制");
    auto actPaste = menu_edit->addAction(menuIcon("iconPaste.png"), "粘贴");
    menu_edit->addSeparator();
    auto actSelectAll = menu_edit->addAction("全选");
    actUndo->setShortcut(QKeySequence("Ctrl+Z"));
    actRedo->setShortcut(QKeySequence("Ctrl+Y"));
    actDel->setShortcut(QKeySequence("Ctrl+D"));
    actCut->setShortcut(QKeySequence("Ctrl+X"));
    actCopy->setShortcut(QKeySequence("Ctrl+C"));
    actPaste->setShortcut(QKeySequence("Ctrl+V"));
    actSelectAll->setShortcut(QKeySequence("Ctrl+A"));

    actUndo->setShortcutContext(Qt::WidgetShortcut);
    actRedo->setShortcutContext(Qt::WidgetShortcut);
    actDel->setShortcutContext(Qt::WidgetShortcut);
    actCut->setShortcutContext(Qt::WidgetShortcut);
    actCopy->setShortcutContext(Qt::WidgetShortcut);
    actPaste->setShortcutContext(Qt::WidgetShortcut);
    actSelectAll->setShortcutContext(Qt::WidgetShortcut);
    connect(actUndo, &QAction::triggered, this, &MainWindow::onActionUndo);
    connect(actRedo, &QAction::triggered, this, &MainWindow::onActionRedo);
    connect(actDel, &QAction::triggered, this, &MainWindow::onActionDel);
    connect(actCut, &QAction::triggered, this, &MainWindow::onActionCut);
    connect(actCopy, &QAction::triggered, this, &MainWindow::onActionCopy);
    connect(actPaste, &QAction::triggered, this, &MainWindow::onActionPaste);
    connect(actSelectAll, &QAction::triggered, this, &MainWindow::onActionSelectAll);

    QMenu *menu_build = menubar->addMenu("构建");
    auto actBuild = menu_build->addAction("编译");    
    connect(actBuild, &QAction::triggered, this, &MainWindow::onActionBuild);

    QMenu *menu_tool = menubar->addMenu("工具");
    auto actCommTool = menu_tool->addAction("通信");
    connect(actCommTool, &QAction::triggered, this, &MainWindow::onActionCommTool);

    auto actProfile = menu_tool->addAction("Profile");
    connect(actProfile, &QAction::triggered, this, &MainWindow::onActionProfile);

    // menu_help
    QMenu *menu_help = menubar->addMenu("帮助");
    menu_help->setProperty("JZMenuType", Menu_Help);
    auto actHelp = menu_help->addAction("查看帮助");    
    menu_help->addSeparator();    
    auto actSample = menu_help->addAction("生成示例");
    auto actAbout = menu_help->addAction("关于" + windowTitle());        
    connect(actAbout, &QAction::triggered, this, &MainWindow::onActionAbout);
    connect(actHelp, &QAction::triggered, this, &MainWindow::onActionHelp);
    connect(actSample, &QAction::triggered, this, &MainWindow::onActionSample);

    m_menuList << menu_file << menu_edit << menu_help;
    layout->addWidget(menubar);

    QAction *actRunOnce = new QAction(JZVisionUtils::icon("runOnce.png"), "运行一次");
    QAction *actRun = new QAction(JZVisionUtils::icon("run.png"), "连续运行");
    connect(actRunOnce, &QAction::triggered, this, &MainWindow::onActionRunOnce);
    connect(actRun, &QAction::triggered, this, &MainWindow::onActionRun);        
    m_actionRun = actRun;

    //tool bar
    QToolBar *main_tool = new QToolBar();
    main_tool->addAction(actSaveFile);
    main_tool->addAction(actSaveAllFile);
    main_tool->addSeparator();
    main_tool->addAction(actRunOnce);
    main_tool->addAction(actRun);        

    layout->addWidget(main_tool);
    
    setEditorMenuBar(menubar);
    setEditorToolBar(main_tool);
}

void MainWindow::onModifyChanged(bool flag)
{
    auto editor = qobject_cast<JZEditor*>(sender());
    int index = m_editorTab->indexOf(editor);
    if (index == -1)
        return;

    updateTabText(index);
    updateActionStatus();
}

void MainWindow::onRedoAvailable(bool flag)
{
    m_menuList[1]->actions()[1]->setEnabled(flag);
}

void MainWindow::onUndoAvailable(bool flag)
{
    m_menuList[1]->actions()[0]->setEnabled(flag);
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


void MainWindow::addCameraPage()
{
    JZPanelWidget* panel = new JZPanelWidget();
    panel->addTab("设备列表", m_cameraList);

    auto log = new JZLogWidget();
    m_mainLog = log;

    QSplitter* splitterRight = new QSplitter(Qt::Vertical);    
    splitterRight->addWidget(m_cameraView);
    splitterRight->addWidget(new JZDockWidget("日志", log));
    splitterRight->setChildrenCollapsible(false);
    splitterRight->setSizes({ 600,200 });
    splitterRight->setStretchFactor(0, 0);
    splitterRight->setStretchFactor(1, 1);
    setSliderStyle(splitterRight);

    QSplitter* splitterTop = new QSplitter(Qt::Horizontal);
    splitterTop->addWidget(panel);
    splitterTop->addWidget(splitterRight);
    splitterTop->setSizes({ 200,600 });
    splitterTop->setStretchFactor(0, 0);
    splitterTop->setStretchFactor(1, 1);

    splitterTop->setChildrenCollapsible(false);
    setSliderStyle(splitterTop);
    m_stack->addWidget(splitterTop);
}

void MainWindow::addFlowPage()
{
    m_projectTree = new JZFlowTree();

    JZPanelWidget* panel = new JZPanelWidget();
    panel->addTab("流程列表", m_projectTree);

    m_buildLog = new LogWidget();
    connect(m_buildLog, &LogWidget::sigNavigate, this, &MainWindow::onNavigate);

    connect(m_projectTree, &JZProjectTree::sigActionTrigged, this, &MainWindow::onProjectTreeAction);

    QWidget* widget = new QWidget();
    QVBoxLayout* center = new QVBoxLayout();
    center->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(center);

    QWidget* widget_left = new QWidget();
    QVBoxLayout* l_left = new QVBoxLayout();
    l_left->setContentsMargins(0, 0, 0, 0);
    widget_left->setLayout(l_left);

    //main
    QSplitter* splitterMain = new QSplitter(Qt::Horizontal);
    splitterMain->addWidget(panel);
    splitterMain->addWidget(widget_left);

    center->addWidget(splitterMain);

    //left    
    m_editorTab = new QTabWidget();
    m_editorTab->setTabsClosable(true);
    connect(m_editorTab, &QTabWidget::tabCloseRequested, this, &MainWindow::onEditorClose);
    connect(m_editorTab, &QTabWidget::currentChanged, this, &MainWindow::onEditorActivity);

    m_editorTab->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editorTab->tabBar(), &QWidget::customContextMenuRequested, this, &MainWindow::onTabContextMenu);

    QSplitter* splitterLeft = new QSplitter(Qt::Vertical);
    splitterLeft->addWidget(m_editorTab);
    splitterLeft->addWidget(m_buildLog);
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

    m_stack->addWidget(widget);
}

void MainWindow::addModelPage()
{
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    QWidget* w = new QWidget();
    w->setLayout(l);

    QLabel* title = new QLabel("模型列表");
    title->setStyleSheet("background-color: rgb(77,96,130); color: rgb(255,255,255);");
    title->setFixedHeight(TITLE_H);
    l->addWidget(title);

    JZModelConfigWidget* config = new JZModelConfigWidget();
    m_modelConfigWidget = config;
    connect(config, &JZModelConfigWidget::sigModelChanged, this, &MainWindow::onModelConfigChanged);

    l->addWidget(config);
    m_stack->addWidget(w);
}

void MainWindow::addCommPage()
{
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    QWidget* w = new QWidget();
    w->setLayout(l);

    QLabel* title = new QLabel("通信列表");
    title->setStyleSheet("background-color: rgb(77,96,130); color: rgb(255,255,255);");
    title->setFixedHeight(TITLE_H);
    l->addWidget(title);

    JZCommConfigWidget* config = new JZCommConfigWidget();    
    m_commConfigWidget = config;
    connect(config, &JZCommConfigWidget::sigCommChanged, this, &MainWindow::onCommConfigChanged);

    l->addWidget(config);
    m_stack->addWidget(w);
}

void MainWindow::addLogPage()
{
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    QWidget* w = new QWidget();
    w->setLayout(l);

    QLabel* title = new QLabel("日志列表");
    title->setStyleSheet("background-color: rgb(77,96,130); color: rgb(255,255,255);");
    title->setFixedHeight(TITLE_H);
    l->addWidget(title);

    JZEventWidget* log = new JZEventWidget();
    l->addWidget(log);
    m_stack->addWidget(w);
}

void MainWindow::addSettingPage()
{
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    QWidget* w = new QWidget();
    w->setLayout(l);

    QLabel* title = new QLabel("设置");
    title->setStyleSheet("background-color: rgb(77,96,130); color: rgb(255,255,255);");
    title->setFixedHeight(TITLE_H);
    l->addWidget(title);

    QWidget* set = new QWidget();
    l->addWidget(set);
    m_stack->addWidget(w);
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

void MainWindow::onBtnLog()
{
    m_stack->setCurrentIndex(4);
}

void MainWindow::onBtnSetting()
{
    m_stack->setCurrentIndex(5);
}

void MainWindow::onActionNewProject()
{
    if (!closeProject())
        return;

    JZNewProjectDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString name = dialog.name();
    QString project_dir = dialog.dir() + "/" + name;
    if (!QDir().exists(project_dir))
        QDir().mkpath(project_dir);

    QString project_path = project_dir + "/" + name + ".jzproj";
    QString project_tmp = dialog.projectType();
    
    initProject();
    if (!m_project.saveAs(project_path) || !m_project.saveAllItem())
    {
        QMessageBox::information(this, "", "新建工程失败");
        return;
    }
    openProject(project_path);
}

void MainWindow::onActionOpenProject()
{
    QString filepath = QFileDialog::getOpenFileName(this, "", "", "*.jzproj");
    if (filepath.isEmpty())
        return;

    if (!closeProject())
        return;

    openProject(filepath);
}

void MainWindow::onActionCloseProject()
{
    closeProject();
}

void MainWindow::onActionRecentProject()
{
    QAction* act = qobject_cast<QAction*>(sender());
    QString filepath = act->text();
    if (!closeProject())
        return;

    if (!openProject(filepath))
    {
        m_setting.recentFile.removeAll(filepath);
        QMenu* menu = qobject_cast<QMenu*>(act->parent());
        menu->removeAction(act);
        act->deleteLater();
    }
}

void MainWindow::onActionSaveFile()
{
    if (!m_editor)
        return;

    m_editor->save();
    updateActionStatus();
}

void MainWindow::onActionCloseFile()
{
    if (!m_editor)
        return;

    closeEditor(m_editor);
    updateActionStatus();
}

void MainWindow::onActionSaveAllFile()
{
    saveAll();
    updateActionStatus();
}

void MainWindow::onActionCloseAllFile()
{
    closeAllEditor();
}

void MainWindow::onActionCloseAllFileExcept()
{
    closeAllEditor(m_editor);
}

void MainWindow::onComToolClose()
{
    JZCommSimulator *sim = qobject_cast<JZCommSimulator*>(sender());
    auto cfg = sim->config();
    m_dbConfig.setConfig("JZModbusSimulatorConfig", cfg);
}

void MainWindow::onFloatWindowDestory()
{
    QObject *obj = sender();
    if (obj == m_traceView)
        m_traceView = nullptr;

    for (int i = 0; i < m_floatWindow.size(); i++)
    {
        if (m_floatWindow[i] == obj)
            m_floatWindow.removeAt(i);
    }
}

void MainWindow::addFlowWindow(QWidget *w)
{
    connect(w, &QWidget::destroyed, this, &MainWindow::onFloatWindowDestory );
    m_floatWindow << w;
}

void MainWindow::onActionCommTool()
{
    JZCommSimulator *simulator = new JZCommSimulator();    
    connect(simulator, &JZCommSimulator::sigClose, this, &MainWindow::onComToolClose);

    JZCommSimulatorConfig cfg = m_dbConfig.getConfig<JZCommSimulatorConfig>("JZModbusSimulatorConfig");
    simulator->setConfig(cfg);
    simulator->show();
    addFlowWindow(simulator);
}

void MainWindow::onActionProfile()
{
    if (!m_traceView)
    {
        m_traceView = new JZNodeTraceView();
        m_traceView->show();
        addFlowWindow(m_traceView);
    }
    else
    {
        m_traceView->show();
    }
    QList<JZNodeTraceItem> trace_items = m_engine.currentTraceContext()->parse();
    m_traceView->setTraceData(trace_items);
    m_traceView->resize(800, 600);
}

void MainWindow::onActionUndo()
{
    if (m_editor)
        m_editor->undo();
}

void MainWindow::onActionRedo()
{
    if (m_editor)
        m_editor->redo();
}

void MainWindow::onActionDel()
{
    if (m_editor)
        m_editor->remove();
}

void MainWindow::onActionCut()
{
    if (m_editor)
        m_editor->cut();
}

void MainWindow::onActionCopy()
{
    if (m_editor)
        m_editor->copy();
}

void MainWindow::onActionPaste()
{
    if (m_editor)
        m_editor->paste();
}

void MainWindow::onActionSelectAll()
{
    if (m_editor)
        m_editor->selectAll();
}

void MainWindow::onActionAbout()
{    
    JZAboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::onActionSample()
{
    initProject();

    JZVisionAppSample sample;
    sample.create(&m_project);

    QString default_dir = qApp->applicationDirPath() + "/project/default/default.proj";
    sample.save(default_dir);
}

void MainWindow::onActionHelp()
{
    QString url = "https://www.juzisoftware.cn/JZVision/index.html";
    QDesktopServices::openUrl(url);
}

void MainWindow::onActionBuild()
{
    m_task.addBuildProgramTask(true);
}

void MainWindow::onActionRun()
{
    if (!isRun())
    {        
        if (!checkBuild())
            return;

        if (m_stack->currentIndex() == 0)
        {
            auto camera_list = m_cameraManager->cameraList();
            for (int i = 0; i < camera_list.size(); i++)
                startCamera(camera_list[i]->name());

            m_running = true;
            updateActionStatus();
        }
        else if (m_stack->currentIndex() == 1)
        {
            onAutoRun();
        }
    }
    else
    {
        stop();                
    }
}

void MainWindow::onActionRunOnce()
{
    if (!checkBuild())
        return;

    if (m_stack->currentIndex() == 0)
    {        
        auto camera_list = m_cameraManager->cameraList();
        for (int i = 0; i < camera_list.size(); i++)
            startCameraOnce(camera_list[i]->name());
    }
    else if (m_stack->currentIndex() == 1)
    {
        onAutoRunOnce();
    }
}

void MainWindow::onEditorClose(int index)
{
    JZEditor* editor = qobject_cast<JZEditor*>(m_editorTab->widget(index));
    closeEditor(editor);
}

void MainWindow::onEditorActivity(int index)
{
    if (index == -1)
        return;

    JZEditor* editor = qobject_cast<JZEditor*>(m_editorTab->widget(index));
    switchEditor(editor);
}

void MainWindow::onNavigate(QUrl url)
{
    QString path = url.path();
    if (openEditor(path))
    {
        m_editor->navigate(url);
    }
}

void MainWindow::onCameraConfigChanged()
{
    JZCameraManagerConfig cfg = m_cameraManager->config();
    JZNodeCameraInit *node = dynamic_cast<JZNodeCameraInit*>(getInitNode(Node_CameraInit));
    node->setConfig(cfg);
    m_project.saveItem(node->file());
}

void MainWindow::onModelConfigChanged()
{
    JZModelConfigWidget* config_widget = dynamic_cast<JZModelConfigWidget*>(sender());    

    JZModelManagerConfig cfg = config_widget->config();
    m_modelManager->setConfig(cfg);
    auto *node = dynamic_cast<JZNodeModelInit*>(getInitNode(Node_ModelInit));
    node->setConfig(cfg);
    m_project.saveItem(node->file());
}

void MainWindow::onCommConfigChanged()
{
    JZCommConfigWidget* config_widget = dynamic_cast<JZCommConfigWidget*>(sender());

    JZCommManagerConfig cfg = config_widget->config();
    m_commManager->setConfig(cfg);
    auto *node = dynamic_cast<JZNodeCommInit*>(getInitNode(Node_CommInit));
    node->setConfig(cfg);
    m_project.saveItem(node->file());
}

void MainWindow::onProjectItemChanged(JZProjectItem* item)
{
    //editor
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        int index = m_editorTab->indexOf(it.value());
        updateTabText(index);

        it++;
    }
    onProjectChanged();
}

void MainWindow::onProjectChanged()
{
    releaseEngine();
    onActionBuild();
}

void MainWindow::onProjectTreeAction(int type, QString filepah)
{
    if (type == Action_open)
        openItem(filepah);
    else if (type == Action_close)
        closeItem(filepah);
    else if (type == Action_remove)
        removeItem(filepah);
}

void MainWindow::onTabContextMenu(QPoint pos)
{
    QMenu menu(this);
    QAction* actSave = menu.addAction("保存");
    QAction* actClose = menu.addAction("关闭");
    QAction* actAll = menu.addAction("关闭所有文档");
    QAction* actAllExcept = menu.addAction("除此之外全部关闭");

    auto bar = qobject_cast<QTabBar*>(sender());
    QAction* ret = menu.exec(bar->mapToGlobal(pos));
    if (!ret)
        return;
    if (ret == actSave)
    {
        onActionSaveFile();
    }
    else if (ret == actClose)
    {
        onActionCloseFile();
    }
    else if (ret == actAll)
    {
        onActionCloseAllFile();
    }
    else if (ret == actAllExcept)
    {
        int index = bar->tabAt(pos);
        auto editor = qobject_cast<JZEditor*>(m_editorTab->widget(index));
        closeAllEditor(editor);
    }
}

void MainWindow::onFunctionOpen(QString functionName)
{
    auto file = m_project.functionItem(functionName);
    if(file)
        openEditor(file->itemPath());
}

void MainWindow::onAutoCompiler()
{
    m_task.addAutoCompilerTask();
}

void MainWindow::onAutoRun()
{
    if (!m_engine.isInit())
        return;

    if (!isCameraFlow())
    {
        LOG_W("请选择要执行的流程");
        return;
    }

    JZNodeCameraReadyEvent *node = currrentCameraNode();
    QString camera_name = node->camera();
    startCamera(camera_name);
    m_running = true;
    updateActionStatus();
}

void MainWindow::onAutoRunOnce()
{
    if (!m_engine.isInit())
        return;

    if (!isCameraFlow())
    {
        LOG_W("请选择要执行的流程");
        return;
    }

    auto file = currentNodeEditor()->view()->file();

    JZEngineTraceConfig trace;
    auto node_list = file->nodeList();
    for(int i = 0; i < node_list.size(); i++)
    {
        auto node = file->getNode(node_list[i]);
        if(node->isFlowNode())
            trace.nodeList << node->id();
    }
    m_engine.setNodeTrace(trace);

    JZNodeCameraReadyEvent *node = currrentCameraNode();
    QString camera_name = node->camera();
    startCameraOnce(camera_name);
}

void MainWindow::onAutoRunStop()
{
    stop();
}

void MainWindow::onBuildStart()
{
    m_buildLog->clearLog(Log_Compiler);
}

void MainWindow::onBuildFinish(JZNodeBuildResultPtr result)
{
    releaseEngine();
    m_buildResult = result;

    auto editor = currentNodeEditor();
    if (editor)
        editor->view()->updateNodeName();

    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        if (it.value()->type() == ProjectItem_scriptItem)
        {
            auto node_edit =  qobject_cast<JZVisionEditor*>(it.value());
            auto cmp_info = compilerResult(it.key()->itemPath());
            if (cmp_info)
                node_edit->setCompilerResult(cmp_info);
        }
        it++;
    }

    if (m_buildResult->status == Build_Successed)
    {        
        initEngine();        
    }
    else
    {
        LOG_W("编译失败，请检查编译结果");
    }
}

void MainWindow::onBuildLog(int level, QString text)
{
    m_buildLog->addLog(Log_Compiler, text);
}

void MainWindow::onAutoRunResult(int result)
{
    auto engine = m_task.runThread()->engine();
}

void MainWindow::dealCameraEvent(QString function,cv::Mat mat)
{
    auto obj_inst = m_engine.environment()->objectManager();
    JZNodeObjectPointer mat_ptr = obj_inst->objectReferencePointer(&mat, false);

    QVariantList in, out;
    in << QVariant::fromValue(m_app) << QVariant::fromValue(mat_ptr);
    m_engine.call(function, in, out);
}

void MainWindow::onFrameReady(QString camera, cv::Mat mat)
{       
    if (!m_engine.isInit())
        return;

    auto it = m_camProgram.find(camera);
    if (it == m_camProgram.end())
        return;        

    QString function = it->function;
    for (int i = 0; i < m_coroutine.size(); i++)
    {
        auto co = dynamic_cast<JZVisionCoroutine*>(m_coroutine[i].data());
        if(co->function == function)
        {
            LOG_W(camera + " drop frame");
            return;
        }
    }

    if (m_stack->currentIndex() == 0)
    {
        m_cameraView->label(camera)->setImage(QtOcv::mat2Image(mat));
    }
    else
    {
        JZNodeCameraReadyEvent* camera_event = currrentCameraNode();
        if (camera_event)
        {
            if (camera_event->camera() != camera)
                return;

            currentNodeEditor()->clearRuntimeResult();
        }
    }

    auto vision_co = new JZVisionCoroutine(&m_engine);
    vision_co->function = function;

    JZCoCoroutinePtr ptr = JZCoCoroutinePtr(vision_co);
    m_coroutine << ptr;
    ptr->setTask([this, function, mat, ptr] { 
        dealCameraEvent(function, mat);
        m_coroutine.removeAll(ptr);
    });
    jzco_spawn(ptr);
}

void MainWindow::onCameraError(QString camera, QString error)
{
    LOG_W(camera + " " + error);
}

void MainWindow::onMainStackedChanged()
{
    stop();
}

void MainWindow::onRuntimeError(JZNodeRuntimeError error)
{
    clearEnv();
    releaseEngine();
    QMessageBox::information(this, "", error.errorReport());
}

void MainWindow::onEngineStatusChanged(JZEngineStatus status)
{
    updateActionStatus();
}

void MainWindow::onNodeTrace(const NodeTraceInfo& trace)
{
    auto editor = currentNodeEditor();
    if (!editor)
        return;

    m_buildLog->addLog(Log_Runtime, trace.toString());
}

void MainWindow::clearEnv()
{
    m_commManager->closeAll();    
}

bool MainWindow::initEngine()
{
    if (m_engine.isInit())
        return true;

    m_engine.setProgram(&m_buildResult->program);
    if (!m_engine.init())
        return false;

    auto obj_inst = m_engine.environment()->objectManager();
    m_app = obj_inst->createHolder(m_className);

    JZVisionApplication *app = JZObjectCast<JZVisionApplication>(m_app.object());
    app->setMainWindow(this);

    m_camProgram.clear();
    auto cls_item = m_project.getClass(m_className);
    auto functions = cls_item->flowList();
    for (int i = 0; i < functions.size(); i++)
    {
        auto flow = cls_item->flow(functions[i]);
        auto event = flow->startNode();
        if (!event || event->type() != Node_CameraFrameReady)
            continue;

        auto cam_event = dynamic_cast<JZNodeCameraReadyEvent*>(event);
        auto camera_name = cam_event->camera();

        CameraProgram prog;
        prog.function = cam_event->function().fullName();
        m_camProgram[camera_name] = prog;
    }

    return true;
}

void MainWindow::releaseEngine()
{
    auto camera_list = m_cameraManager->cameraList();
    for (int i = 0; i < camera_list.size(); i++)
        stopCamera(camera_list[i]->name());

    if (m_engine.isInit())
        m_engine.deinit();

    m_app.relaseObject();
}

void MainWindow::initProject()
{
    m_project.clear();

    JZProjectTemplate::instance()->initProject(&m_project, "empty");    

    JZScriptClassItem *class_item = m_project.mainFile()->addClass(m_className,"JZVisionApplication");
    JZFunctionDefine func_init_def = class_item->objectDefine().initMemberFunction("init");
    JZScriptItem *script = class_item->addMemberFunction(func_init_def);
    auto start = script->startNode();
    JZNodeCameraInit * camera_init = new JZNodeCameraInit();
    JZNodeCommInit* comm_init = new JZNodeCommInit();
    JZNodeModelInit* model_init = new JZNodeModelInit();
    script->addNode(camera_init);
    script->addNode(model_init);
    script->addNode(comm_init);
    script->addConnect(start->flowOutGemo(), camera_init->flowInGemo());
    script->addConnect(camera_init->flowOutGemo(), model_init->flowInGemo());
    script->addConnect(model_init->flowOutGemo(), comm_init->flowInGemo());

    JZEditorUtils::projectUpdateLayout(&m_project);
}

bool MainWindow::closeProject()
{
    if (!closeAllEditor())
        return false;

    m_buildLog->clearLogs();
    m_projectTree->clear();
    m_project.close();
    updateActionStatus();
    setWindowTitle("JZVison");
    return true;
}

bool MainWindow::openProject(QString filepath)
{
    if (!m_project.open(filepath))
    {
        QMessageBox::information(this, "", "打开工程失败: " + m_project.error());
        return false;
    }

    m_projectTree->setProject(&m_project);
    m_setting.addRecentProject(m_project.filePath());
    updateActionStatus();
    setWindowTitle(m_project.name() + "- JZVison");

    auto *node_camera = dynamic_cast<JZNodeCameraInit*>(getInitNode(Node_CameraInit));
    auto *node_model = dynamic_cast<JZNodeModelInit*>(getInitNode(Node_ModelInit));
    auto *node_comm = dynamic_cast<JZNodeCommInit*>(getInitNode(Node_CommInit));
    m_cameraManager->setConfig(node_camera->config());
    m_modelManager->setConfig(node_model->config());
    m_commManager->setConfig(node_comm->config());

    m_cameraList->updateCamera();
    m_modelConfigWidget->setConfig(node_model->config());
    m_commConfigWidget->setConfig(node_comm->config());

    onActionBuild();
    return true;
}

void MainWindow::openItem(QString filepath)
{
    openEditor(filepath);
}

void MainWindow::closeItem(QString filepath)
{
    auto edit = editor(filepath);
    if (!edit)
        return;

    closeEditor(edit);
}

void MainWindow::removeItem(QString itempath)
{
    auto item = m_project.getItem(itempath);
    auto file_item = m_project.getItemFile(item);

    closeItem(itempath);
    m_project.removeItem(itempath);
}

JZEditor* MainWindow::createEditor(int type)
{
    JZEditor* editor = JZEditorManager::instance()->createEditor(type);
    if (editor)
    {        
        editor->setProject(&m_project);
    }
    return editor;
}

bool MainWindow::openEditor(QString filepath)
{
    if (filepath == "__idle__")
        return false;

    JZProjectItem* item = m_project.getItem(filepath);
    if (!item)
        return false;

    QString file = item->itemPath();
    auto new_edit = editor(file);
    if (!new_edit) {
        new_edit = createEditor(item->itemType());
        if (!new_edit)
            return false;

        connect(new_edit, &JZEditor::redoAvailable, this, &MainWindow::onRedoAvailable);
        connect(new_edit, &JZEditor::undoAvailable, this, &MainWindow::onUndoAvailable);
        connect(new_edit, &JZEditor::modifyChanged, this, &MainWindow::onModifyChanged);
        new_edit->setItem(item);
        new_edit->open(item);
        if (new_edit->type() == ProjectItem_scriptItem)
        {
            auto node_edit =  qobject_cast<JZVisionEditor*>(new_edit);
            connect(node_edit, &JZVisionEditor::sigAutoCompiler, this, &MainWindow::onAutoCompiler);
            connect(node_edit, &JZVisionEditor::sigAutoRunOnce, this, &MainWindow::onAutoRunOnce);
            connect(node_edit, &JZVisionEditor::sigAutoRun, this, &MainWindow::onAutoRun);
            connect(node_edit, &JZVisionEditor::sigAutoRunStop, this, &MainWindow::onAutoRunStop);

            auto cmp_ret = compilerResult(file);
            if (cmp_ret)
                node_edit->setCompilerResult(cmp_ret);
        }

        m_editors[item] = new_edit;
        m_editorTab->addTab(new_edit, filepath);
    }
    switchEditor(new_edit);

    return true;
}

void MainWindow::closeEditor(JZEditor* editor)
{
    auto item = editor->item();
    if (editor->isModified())
    {
        int ret = QMessageBox::question(this, "", "是否保存", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Yes)
        {
            editor->save();
        }
        else if (ret == QMessageBox::No)
        {
        }
        else if (ret == QMessageBox::Cancel)
        {
            return;
        }
    }
    editor->close();
    editor->setItem(nullptr);

    m_editors.remove(item);
    if (m_editor == editor)
    {
        if (m_editors.size() > 0)
            switchEditor(m_editors.first());
        else
            switchEditor(nullptr);
    }

    int index = m_editorTab->indexOf(editor);
    m_editorTab->removeTab(index);
    delete editor;
}

JZEditor* MainWindow::editor(QString filepath)
{
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        if (it.key()->itemPath() == filepath)
            return it.value();

        it++;
    }
    return nullptr;
}

JZVisionEditor* MainWindow::currentNodeEditor()
{
    if (m_editor && m_editor->type() == ProjectItem_scriptItem)
        return qobject_cast<JZVisionEditor*>(m_editor);
    else
        return nullptr;
}

QList<JZVisionEditor*> MainWindow::nodeEditorList()
{
    QList<JZVisionEditor*> list;
    //editor
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (it.value()->type() == ProjectItem_scriptItem)
        {
            auto node_edit = qobject_cast<JZVisionEditor*>(it.value());
            list << node_edit;
        }
        it++;
    }
    return list;
}

JZVisionEditor* MainWindow::nodeEditor(QString filepath)
{
    JZEditor* e = editor(filepath);
    if (!e)
        return nullptr;

    return qobject_cast<JZVisionEditor*>(e);
}

void MainWindow::switchEditor(JZEditor* editor)
{
    if (editor == m_editor)
        return;

    if (m_editor)
    {
        m_editor->inactive‌();
        stop();
    }

    m_editor = editor;
    if (editor != nullptr)
    {
        m_editorTab->setCurrentWidget(m_editor);
        m_editor->active();
        m_editor->setFocus();
    }
    else
        m_editorTab->setCurrentIndex(0);
    updateActionStatus();
}

bool MainWindow::closeAllEditor(JZEditor* except)
{
    QList<JZEditor*> close_list;

    m_project.saveTransaction();
    bool saveToAll = false, noToAll = false;
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (except && except == editor)
        {
            it++;
            continue;
        }

        if (editor->isModified())
        {
            if (!saveToAll && !noToAll)
            {
                int ret = QMessageBox::question(this, "", "是否保存", QMessageBox::Yes | QMessageBox::No
                    | QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel);
                if (ret == QMessageBox::Yes || ret == QMessageBox::YesToAll)
                {
                    editor->save();
                    if (ret == QMessageBox::YesToAll)
                        saveToAll = true;
                }
                else if (ret == QMessageBox::No || ret == QMessageBox::NoToAll)
                {
                    if (ret == QMessageBox::NoToAll)
                        noToAll = true;
                }
                else if (ret == QMessageBox::Cancel)
                {
                    m_project.saveCommit();
                    return false;
                }
            }
            else if (saveToAll)
            {
                editor->save();
            }
            else if (noToAll)
            {

            }
        }
        editor->close();
        close_list << editor;

        it++;
    }
    m_project.saveCommit();

    for (auto editor : close_list)
    {
        int index = m_editorTab->indexOf(editor);
        m_editorTab->removeTab(index);
        m_editors.remove(editor->item());
        delete editor;
    }

    m_editor = nullptr;
    switchEditor(except);
    return true;
}

void MainWindow::saveAll()
{
    m_project.saveTransaction();
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (editor->isModified())
            editor->save();

        it++;
    }
    m_project.saveCommit();
}

void MainWindow::updateActionStatus()
{
    if (m_running)
    {
        m_actionRun->setIcon(JZVisionUtils::icon("stop.png"));
        m_actionRun->setText("停止");;
    }
    else
    {
        m_actionRun->setIcon(JZVisionUtils::icon("run.png"));
        m_actionRun->setText("运行");
    }
}

void MainWindow::updateTabText(int index)
{
    auto editor = qobject_cast<JZEditor*>(m_editorTab->widget(index));
    QString title = editor->item()->itemPath();
    if (editor->isModified())
        title += "*";
    m_editorTab->setTabText(index, title);
}

JZNode *MainWindow::getInitNode(int type)
{
    auto cls_item = m_project.getClass(m_className);
    auto init_script = cls_item->memberFunction("init");
    auto node_list = init_script->findNodeByType(type);
    Q_ASSERT(node_list.size() == 1);
    return node_list[0];
}

bool MainWindow::checkBuild()
{
    bool flag = true;
    if (!m_buildResult || m_buildResult->status != Build_Successed)
        flag = false;
    else
    {
        flag = initEngine();
    }

    if (!flag)
    {
        QMessageBox::information(this, "", "初始化失败，请检查流程或重新编译");
        return false;
    }

    return true;
}

bool MainWindow::isCameraFlow()
{
    return currrentCameraNode() != nullptr;
}

JZCamera *MainWindow::camera(QString name)
{
    return m_cameraManager->camera(name);
}

bool MainWindow::openCamera(JZCamera* camera)
{
    if (!camera->isOpen())
    {
        if (!camera->open())
        {
            QMessageBox::information(this, "", "启动相机失败: " + camera->error());
            return false;
        }
    }
    return true;
}

void MainWindow::startCamera(QString name)
{
    JZCamera *camera = this->camera(name);
    if (!openCamera(camera))
        return;

    camera->start();
    LOG_I("相机启动");
}

void MainWindow::startCameraOnce(QString name)
{
    JZCamera *camera = this->camera(name);
    if (!openCamera(camera))
        return;

    camera->startOnce();
    LOG_I("相机启动一次");
}

void MainWindow::stopCamera(QString name)
{
    JZCamera *camera = this->camera(name);
    camera->close();    

    LOG_I("相机停止");
}

bool MainWindow::isRun()
{
    return m_running;
}

void MainWindow::stop()
{
    auto camera_list = m_cameraManager->cameraList();
    for (int i = 0; i < camera_list.size(); i++)
        stopCamera(camera_list[i]->name());    

    if(m_engine.isInit())
        m_engine.stopAllCo();    

    m_running = false;
}

void MainWindow::imageDebug()
{    
    auto env = m_engine.environment();
    int node_id = m_engine.getReg(Reg_CallIn).toInt();
    QVariant v = m_engine.getReg(Reg_CallIn + 1);
    auto mat = JZObjectCast<cv::Mat>(toJZObject(v));

    ImageResult image;
    image.mat = *mat;
    if (m_engine.regInCount() == 3)
    {
        QVariant roi = m_engine.getReg(Reg_CallIn + 2);
        QString roi_type = env->variantTypeName(roi);

        if(m_roiFunction.contains(roi_type))
        { 
            QString roi_func_name = m_roiFunction[roi_type];
            const JZFunction *roi_func = env->functionImpl(roi_func_name);
        
            QVariantList in, out;
            in << roi;
            roi_func->cfunc->call(in,out);
       
            QList<JZGraphic> *graphic_list = JZObjectCast<QList<JZGraphic>>(toJZObject(out[0]));
            image.graphList = *graphic_list;
        }
    }

    if (m_stack->currentIndex() == 0)
    {
        QString function = m_engine.runtimeInfo().stacks[0].function;
        QString camera = getCameraByProgram(function);
        if (camera.isEmpty())
            return;

        m_cameraView->label(camera)->setGraphics(image.graphList);
        return;
    }

    JZNodeCameraReadyEvent* camera_event = currrentCameraNode();
    if (!camera_event)
        return;    
    
    NodeResult result;
    result.outputImage << image;
    
    auto editor = currentNodeEditor();
    editor->setRuntimeResult(node_id, result);
}

QString MainWindow::getCameraByProgram(QString function)
{
    auto it = m_camProgram.begin();
    while (it != m_camProgram.end())
    {
        if (it->function == function)
            return it.key();

        it++;
    }
    return QString();
}

JZNodeCameraReadyEvent* MainWindow::currrentCameraNode()
{
    auto editor = currentNodeEditor();
    if (!editor)
        return nullptr;

    auto script = dynamic_cast<JZScriptItem*>(editor->item());
    auto event = script->startNode();
    if (!event || event->type() != Node_CameraFrameReady)
        return nullptr;

    return dynamic_cast<JZNodeCameraReadyEvent*>(event);
}

const CompilerResult* MainWindow::compilerResult(const QString& path)
{
    if (!m_buildResult)
        return nullptr;

    auto it = m_buildResult->compilerResult.find(path);
    if (it == m_buildResult->compilerResult.end())
        return nullptr;

    return &it.value();
}