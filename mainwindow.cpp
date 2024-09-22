#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QCoreApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QElapsedTimer>
#include <QToolBar>
#include "JZUiEditor.h"
#include "JZNodeParamEditor.h"
#include "JZNewProjectDialog.h"
#include "JZDesignerEditor.h"
#include "JZNodeUtils.h"
#include "JZNodeCppGenerater.h"
#include "3rd/jzupdate/JZUpdateClient.h"
#include "3rd/jzupdate/JZUpdateDialog.h"
#include "JZAboutDialog.h"
#include "JZProjectSettingDialog.h"
#include "JZNodeParamDelegate.h"

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

QDataStream &operator<<(QDataStream &s, const Setting &param)
{
    s << param.recentFile;
    return s;
}

QDataStream &operator >> (QDataStream &s, Setting &param)
{
    s >> param.recentFile;
    return s;
}


//AutoBuildInfo
MainWindow::BuildInfo::BuildInfo()
{
    clear();
}

void MainWindow::BuildInfo::clear()
{
    changeTimestamp = QDateTime::currentMSecsSinceEpoch();
    buildTimestamp = 0;
    saveTimestamp = 0;
    save = false;
    start = false;
    success = false;
    runItemPath.clear();
}

void MainWindow::BuildInfo::clearTask()
{
    save = false;
    start = false;
    runItemPath.clear();
}

bool MainWindow::BuildInfo::isUnitTest()
{
    if(!runItemPath.isEmpty() && !start)
        return true;
    
    return false;
}

//ActionStatus
MainWindow::ActionStatus::ActionStatus(QAction *act, QVector<int> act_flags)
{
    this->action = act;
    this->flags = act_flags;
}

//MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    m_editor = nullptr;
    m_simulator = nullptr;    
    m_processMode = Process_none;
    m_compilerTimer = new QTimer(this);
    connect(m_compilerTimer, &QTimer::timeout, this, &MainWindow::onAutoCompilerTimer);
    m_compilerTimer->start(100);

    connect(LogManager::instance(), &LogManager::sigLog, this, &MainWindow::onLog);

    connect(&m_debuger,&JZNodeDebugClient::sigLog,this,&MainWindow::onRuntimeLog);    
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeError,this,&MainWindow::onRuntimeError);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeStatus, this, &MainWindow::onRuntimeStatus);    
    connect(&m_debuger,&JZNodeDebugClient::sigNetError, this, &MainWindow::onNetError);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeWatch, this, &MainWindow::onRuntimeWatch);

    connect(&m_process,(void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,this,&MainWindow::onRuntimeFinish);
    connect(&m_project,&JZProject::sigItemChanged, this, &MainWindow::onProjectItemChanged);
    connect(&m_project,&JZProject::sigDefineChanged, this, &MainWindow::onProjectChanged);
    connect(&m_project,&JZProject::sigBreakPointChanged, this, &MainWindow::onBreakPointChanged);

    connect(&m_runThread,&JZNodeAutoRunThread::sigResult,this, &MainWindow::onAutoRunResult);
    connect(&m_buildThread,&JZNodeBuildThread::sigResult,this, &MainWindow::onBuildFinish);
    m_buildThread.init(&m_project,&m_program);

    auto engine = m_runThread.engine();
    connect(engine,&JZNodeEngine::sigWatchNotify,this,&MainWindow::onWatchNotify,Qt::BlockingQueuedConnection);

    loadSetting();    
    initUi();     
    updateActionStatus();    
        
    //initLocalProcessTest();
    JZProject::setActive(&m_project);
}

MainWindow::~MainWindow()
{
    for(auto edit : m_editors)
        edit->disconnect();
    
    saveSetting();
}

void MainWindow::loadSetting()
{
    QString path = qApp->applicationDirPath() + "/setting.dat";
    QFile file(path);
    if (file.open(QFile::ReadOnly))
    {
        QDataStream s(&file);
        s >> m_setting;
        file.close();
    }
}

void MainWindow::saveSetting()
{
    QString path = qApp->applicationDirPath() + "/setting.dat";
    QFile file(path);
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QDataStream s(&file);
        s << m_setting;
        file.close();
    }
}

QIcon MainWindow::menuIcon(const QString &name)
{
    return QIcon(":/JZNodeEditor/Resources/icons/" + name);
}

void MainWindow::initMenu()
{
    using as = ActionStatus;

    QMenuBar *menubar = new QMenuBar();
    this->setMenuBar(menubar);

    QMenu *menu_file = menubar->addMenu("文件");
    auto actNewMenu = menu_file->addMenu("新建");
    auto actOpenMenu = menu_file->addMenu("打开");

    auto actNewProject = actNewMenu->addAction("项目");
    auto actOpenProject = actOpenMenu->addAction("项目");
    connect(actNewProject, &QAction::triggered, this, &MainWindow::onActionNewProject);
    connect(actOpenProject, &QAction::triggered, this, &MainWindow::onActionOpenProject);
    menu_file->addSeparator();
/*
    QMenu *menu_NewFile = menu_file->addMenu("添加");
    auto actNewEventFile = menu_NewFile->addAction("新建项目");
    auto actNewClass = menu_NewFile->addAction("类");
    auto actNewFunction = menu_NewFile->addAction("函数");
    connect(actNewEventFile, &QAction::triggered, this, &MainWindow::onActionNewEvent);
    connect(actNewClass, &QAction::triggered, this, &MainWindow::onActionNewClass);
    connect(actNewFunction, &QAction::triggered, this, &MainWindow::onActionNewFunction);
    menu_file->addSeparator();
*/
    auto actCloseFile = menu_file->addAction("关闭");
    auto actCloseProject = menu_file->addAction("关闭工程");
    connect(actCloseFile, &QAction::triggered, this, &MainWindow::onActionCloseFile);
    connect(actCloseProject,&QAction::triggered,this,&MainWindow::onActionCloseProject);
    menu_file->addSeparator();    
    
    auto actSaveFile = menu_file->addAction(menuIcon("iconSave.png"), "保存文件");
    auto actSaveAllFile = menu_file->addAction(menuIcon("iconSaveAll.png"), "全部保存");
    auto actCloseAllFile = menu_file->addAction("全部关闭");

    connect(actSaveFile,&QAction::triggered,this,&MainWindow::onActionSaveFile);    
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

    m_actionStatus << ActionStatus(actCloseProject, { as::ProjectVaild })
        << ActionStatus(actSaveFile, { as::FileIsModify })
        << ActionStatus(actCloseFile, { as::FileOpen })
        << ActionStatus(actSaveAllFile, { as::HasModifyFile })
        << ActionStatus(actCloseAllFile, { as::FileOpen });

    QMenu *menu_edit = menubar->addMenu("编辑");
    auto actUndo = menu_edit->addAction(menuIcon("iconUndo.png"),"撤销");
    auto actRedo = menu_edit->addAction(menuIcon("iconRedo.png"),"重做");
    menu_edit->addSeparator();
    auto actDel = menu_edit->addAction(menuIcon("iconDelete.png"),"删除");
    auto actCut = menu_edit->addAction(menuIcon("iconCut.png"),"剪切");
    auto actCopy = menu_edit->addAction(menuIcon("iconCopy.png"),"复制");
    auto actPaste = menu_edit->addAction(menuIcon("iconPaste.png"),"粘贴");
    menu_edit->addSeparator();
    auto actSelectAll = menu_edit->addAction("全选");
    actUndo->setShortcut(QKeySequence("Ctrl+Z"));
    actRedo->setShortcut(QKeySequence("Ctrl+Y"));
    actDel->setShortcut(QKeySequence("Ctrl+D"));
    actCut->setShortcut(QKeySequence("Ctrl+X"));
    actCopy->setShortcut(QKeySequence("Ctrl+C"));
    actPaste->setShortcut(QKeySequence("Ctrl+V"));
    actSelectAll->setShortcut(QKeySequence("Ctrl+A"));

    connect(actUndo,&QAction::triggered,this,&MainWindow::onActionUndo);
    connect(actRedo,&QAction::triggered,this,&MainWindow::onActionRedo);
    connect(actDel,&QAction::triggered,this,&MainWindow::onActionDel);
    connect(actCut,&QAction::triggered,this,&MainWindow::onActionCut);
    connect(actCopy,&QAction::triggered,this,&MainWindow::onActionCopy);
    connect(actPaste,&QAction::triggered,this,&MainWindow::onActionPaste);
    connect(actSelectAll,&QAction::triggered,this,&MainWindow::onActionSelectAll);

    QMenu *menu_view = menubar->addMenu("视图");
    menu_view->addAction("显示窗口");
    menu_view->addAction("恢复默认");

    QMenu *menu_project = menubar->addMenu("项目");
    QAction *actProject = menu_project->addAction("属性");
    connect(actProject, &QAction::triggered, this, &MainWindow::onActionProjectProp);

    QMenu *menu_build = menubar->addMenu("构建");
    auto actBuild = menu_build->addAction("编译");
    auto actExport = menu_build->addAction("导出");
    connect(actBuild,&QAction::triggered,this,&MainWindow::onActionBuild);
    connect(actExport,&QAction::triggered,this,&MainWindow::onActionExport);
    m_actionStatus << ActionStatus(actBuild, { as::ProjectVaild, as::ProcessIsEmpty });

    QMenu *menu_tool = menubar->addMenu("工具");
    auto actModbus = menu_tool->addAction("Modbus");
    connect(actModbus, &QAction::triggered, this, &MainWindow::onActionModbus);

    QMenu *menu_debug = menubar->addMenu("调试");    
    auto actRun = menu_debug->addAction(menuIcon("iconRun.png"), "开始调试");
    auto actDetach = menu_debug->addAction("脱离调试器");
    auto actPause = menu_debug->addAction(menuIcon("iconPause.png"), "中断");
    auto actResume = menu_debug->addAction(menuIcon("iconRun.png"), "继续");
    auto actStop = menu_debug->addAction(menuIcon("iconStop.png"), "停止调试");

    m_debugActions << menu_debug->addSeparator();
    auto actStepOver = menu_debug->addAction(menuIcon("iconStepOver.png"), "单步");
    auto actStepIn = menu_debug->addAction(menuIcon("iconStepIn.png"), "单步进入");
    auto actStepOut = menu_debug->addAction(menuIcon("iconStepOver.png"), "单步跳出");
    auto actBreakPoint = menu_debug->addAction(menuIcon("iconStepBreakpoint.png"), "断点");
    actRun->setShortcut(QKeySequence("F5"));
    actStepOver->setShortcut(QKeySequence("F10"));
    actStepIn->setShortcut(QKeySequence("F11"));
    actStepOut->setShortcut(QKeySequence("Shift+F11"));
    actBreakPoint->setShortcut(QKeySequence("F9"));

    m_debugActions << actDetach << actPause << actResume << actStop << actStepOver 
        << actStepIn << actStepOut;

    m_actionRun = actRun;
    m_actionResume = actResume;

    connect(actRun,&QAction::triggered,this,&MainWindow::onActionRun);
    connect(actDetach,&QAction::triggered,this,&MainWindow::onActionDetach);
    connect(actPause,&QAction::triggered,this,&MainWindow::onActionPause);
    connect(actResume,&QAction::triggered,this,&MainWindow::onActionResume);
    connect(actStop,&QAction::triggered,this,&MainWindow::onActionStop);

    connect(actStepOver,&QAction::triggered,this,&MainWindow::onActionStepOver);
    connect(actStepIn,&QAction::triggered,this,&MainWindow::onActionStepIn);
    connect(actStepOut,&QAction::triggered,this,&MainWindow::onActionStepOut);
    connect(actBreakPoint,&QAction::triggered,this,&MainWindow::onActionBreakPoint);

    m_actionStatus << ActionStatus(actRun, { as::ProjectVaild, as::ProcessIsEmpty })
        << ActionStatus(actDetach, { as::ProcessIsVaild })
        << ActionStatus(actPause, { as::ProcessCanPause })
        << ActionStatus(actResume, { as::ProcessCanResume })
        << ActionStatus(actStop, { as::ProcessIsVaild })
        << ActionStatus(actStepOver, { as::ProcessCanResume })
        << ActionStatus(actStepIn, { as::ProcessCanResume })
        << ActionStatus(actStepOut, { as::ProcessCanResume })
        << ActionStatus(actBreakPoint, { as::FileIsScript });

    QMenu *menu_help = menubar->addMenu("帮助");
    auto actHelp = menu_help->addAction("帮助");
    menu_help->addSeparator();
    auto actCheckUpdate = menu_help->addAction("检查更新");
    auto actAbout = menu_help->addAction("关于" + windowTitle());
    connect(actHelp, &QAction::triggered, this, &MainWindow::onActionHelp);
    connect(actCheckUpdate, &QAction::triggered, this, &MainWindow::onActionCheckUpdate);
    connect(actAbout, &QAction::triggered, this, &MainWindow::onActionAbout);

    m_menuList << menu_file << menu_edit << menu_view << menu_build << menu_debug << menu_help;
    
    //tool bar
    QToolBar *main = new QToolBar();    
    main->addAction(actSaveFile);
    main->addAction(actSaveAllFile);    
    
    QToolBar *tool_debug = new QToolBar();
    tool_debug->addAction(actResume);
    tool_debug->addAction(actPause);    
    tool_debug->addAction(actStop);

    addToolBar(main);
    addToolBar(tool_debug);

    m_toolDebug = tool_debug;
}

void MainWindow::initUi()
{    
    initMenu();    

    m_log = new LogWidget();
    connect(m_log, &LogWidget::sigNavigate, this, &MainWindow::onNavigate);

    m_stack = m_log->stack();    
    connect(m_stack, &JZNodeStack::sigStackChanged, this, &MainWindow::onStackChanged);

    m_watchAuto = m_log->watchAuto();   
    m_watchManual = m_log->watchManual();    
    m_watchAuto->setReadOnly(true);    
    m_debugWidgets << m_watchAuto << m_watchManual;
    for (auto w : m_debugWidgets)
        w->setMainWindow(this);
    
    connect(m_watchManual, &JZNodeWatch::sigParamNameChanged, this, &MainWindow::onWatchNameChanged);
    connect(m_watchManual, &JZNodeWatch::sigParamValueChanged, this, &MainWindow::onWatchValueChanged);

    connect(m_watchAuto, &JZNodeWatch::sigParamValueChanged, this, &MainWindow::onWatchValueChanged);
    
    m_breakPoint = m_log->breakpoint();    
    m_breakPoint->setProject(&m_project);
    connect(m_breakPoint, &JZNodeBreakPoint::sigBreakPointClicked, this, &MainWindow::onBreakPointClicked);    

    m_projectTree = new JZProjectTree();    
    connect(m_projectTree,&JZProjectTree::sigActionTrigged,this,&MainWindow::onProjectTreeAction);

    QWidget *widget = new QWidget();
    QVBoxLayout *center = new QVBoxLayout();     
    center->setContentsMargins(9,9,9,9);
    widget->setLayout(center);

    QWidget *widget_left = new QWidget();
    QVBoxLayout *l_left = new QVBoxLayout();
    l_left->setContentsMargins(0,0,0,0);
    widget_left->setLayout(l_left);

    //main
    QSplitter *splitterMain = new QSplitter(Qt::Horizontal);
    splitterMain->setObjectName("splitterMain");
    splitterMain->addWidget(m_projectTree);
    splitterMain->addWidget(widget_left);

    center->addWidget(splitterMain);    

    //left
    JZNodeEditor *node_editor = new JZNodeEditor();
    m_editorStack = new QTabWidget(); 
    m_editorStack->setTabsClosable(true);
    connect(m_editorStack, &QTabWidget::tabCloseRequested, this, &MainWindow::onEditorClose);
    connect(m_editorStack, &QTabWidget::currentChanged, this, &MainWindow::onEditorActivite);

    m_editorStack->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editorStack->tabBar(), &QWidget::customContextMenuRequested, this, &MainWindow::onTabContextMenu);

    QSplitter *splitterLeft = new QSplitter(Qt::Vertical);
    splitterLeft->addWidget(m_editorStack);    
    splitterLeft->addWidget(m_log);       
    l_left->addWidget(splitterLeft);

    splitterMain->setCollapsible(0,false);
    splitterMain->setCollapsible(1,false);
    splitterMain->setStretchFactor(0,0);
    splitterMain->setStretchFactor(1,1);
    splitterMain->setSizes({250,600});

    splitterLeft->setCollapsible(0,false);
    splitterLeft->setCollapsible(1,false);
    splitterLeft->setStretchFactor(0,1);
    splitterLeft->setStretchFactor(1,0);

    setCentralWidget(widget);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_processMode != Process_none)
    {
        int ret = QMessageBox::question(this, "", "是否停止调试", QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No) 
        {
            event->ignore();
            return;
        }
        onActionStop();
    }
    if (!closeProject())
    {
        event->ignore();
        return;
    }    

    m_runThread.stopRun();
    m_buildThread.stopBuild();

    if (m_simulator)
    {
        m_simulator->closeAll();
        delete m_simulator;
        m_simulator = nullptr;
    }

    JZDesigner::instance()->closeEditor();
    QMainWindow::closeEvent(event);
}

const CompilerResult *MainWindow::compilerResult(const QString &path)
{
    auto s = (JZScriptItem*)m_project.getItem(path);
    return m_buildThread.builder()->compilerInfo(s);
}

JZNodeProgram *MainWindow::program()
{
    return &m_program;
}

JZNodeRuntimeInfo *MainWindow::runtime()
{
    return &m_runtime;
}

int MainWindow::stackIndex()
{
    return m_stack->stackIndex();
}

void MainWindow::updateActionStatus()
{
    using as = ActionStatus;    

    bool isProject = !m_project.isNull();
    bool isEditor = (m_editor != nullptr);
    bool isEditorModify = (m_editor && m_editor->isModified());    
    bool isEditorScript = (m_editor && m_editor->type() == Editor_script);
    bool hasModifyFile = false;
    bool isProcess = m_processMode != Process_none;
    bool canPause = (m_processMode == Process_running);
    bool canResume = (m_processMode == Process_pause);
    for (auto v : m_editors)
    {
        if (v->isModified())
        {
            hasModifyFile = true;
            break;
        }
    }

    QMap<int,bool> cond;
    cond[as::ProjectVaild] = isProject;
    cond[as::FileOpen] = isEditor;
    cond[as::FileIsModify] = isEditorModify;
    cond[as::FileIsScript] = isEditorScript;
    cond[as::HasModifyFile] = hasModifyFile;
    cond[as::ProcessIsEmpty] = !isProcess;
    cond[as::ProcessIsVaild] = isProcess;
    cond[as::ProcessCanPause] = canPause;
    cond[as::ProcessCanResume] = canResume;
    cond[as::ProcessCanStartResume] = canResume || (isProject && !isProcess);
    
    Q_ASSERT(cond.size() == ActionStatus::Count);
    for (int i = 0; i < m_actionStatus.size(); i++)
    {
        auto act = m_actionStatus[i].action;
        auto &flags = m_actionStatus[i].flags;
        int enabled_count = 0;        
        for (int flg_idx = 0; flg_idx < flags.size(); flg_idx++)
        {
            if (cond[flags[flg_idx]])
                enabled_count++;             
        }       

        bool enabled = (enabled_count == flags.size());
        act->setEnabled(enabled);
    }
    
    for (auto act : m_debugActions)
        act->setVisible(isProcess);

    if (canResume)
    {
        m_actionRun->setShortcut(QKeySequence());
        m_actionResume->setShortcut(QKeySequence("F5"));
    }
    else
    {
        m_actionResume->setShortcut(QKeySequence());
        m_actionRun->setShortcut(QKeySequence("F5"));
    }
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
    
    QString project_tmp;
    if (dialog.projectType() == 0)
        project_tmp = "ui";
    else
        project_tmp = "console";

    JZProject project;
    if (!project.newProject(project_dir, name, project_tmp))
        return;
    
    openProject(project.filePath());
}

void MainWindow::onActionOpenProject()
{        
    QString filepath = QFileDialog::getOpenFileName(this,"","","*.jzproj");
    if(filepath.isEmpty())
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
    QAction *act = qobject_cast<QAction*>(sender());
    QString filepath = act->text();
    if (!closeProject())
        return;

    if (!openProject(filepath))
    {             
        m_setting.recentFile.removeAll(filepath);
        QMenu *menu = qobject_cast<QMenu *>(act->parent());
        menu->removeAction(act);  
        delete act;
    }
}

void MainWindow::onActionNewEvent()
{

}

void MainWindow::onActionNewFunction()
{

}

void MainWindow::onActionNewClass()
{    
}

void MainWindow::onActionSaveFile()
{
    if(!m_editor)
        return;

    m_editor->save();    
    updateActionStatus();
}

void MainWindow::onActionCloseFile()
{
    if(!m_editor)
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

void MainWindow::onActionUndo()
{
    if(m_editor)
        m_editor->undo();
}

void MainWindow::onActionRedo()
{
    if(m_editor)
        m_editor->redo();
}

void MainWindow::onActionDel()
{
    if(m_editor)
        m_editor->remove();
}

void MainWindow::onActionCut()
{
    if(m_editor)
        m_editor->cut();
}

void MainWindow::onActionCopy()
{
    if(m_editor)
        m_editor->copy();
}

void MainWindow::onActionPaste()
{
    if(m_editor)
        m_editor->paste();
}

void MainWindow::onActionSelectAll()
{
    if(m_editor)
    {
        m_editor->selectAll();        
    }
}

void MainWindow::onActionProjectProp()
{
    JZProjectSettingDialog dlg(this);
    dlg.setProject(&m_project);
    dlg.exec();
}

void MainWindow::onActionBuild()
{
    m_buildInfo.save = true;
    build();
}

void MainWindow::onActionExport()
{
    JZNodeCppGenerater gen;

    QString output = m_project.path() + "/build/cpp";
    gen.generate(&m_project,output);

    m_log->addLog(Log_Compiler, "export to: " + output);
}

void MainWindow::onActionRun()
{    
    if(m_buildInfo.buildTimestamp >= m_buildInfo.saveTimestamp)
    {
        m_buildInfo.save = true;
        m_buildInfo.start = true;
        build();
    }
    else
    {
        startProgram();
    }    
}

void MainWindow::onActionDetach()
{
    m_debuger.detach();    
    updateActionStatus();
    m_processMode = Process_none;
}

void MainWindow::onActionPause()
{
    m_debuger.pause();
    updateActionStatus();
}

void MainWindow::onActionResume()
{
    m_debuger.resume();
    updateActionStatus();
}

void MainWindow::onActionStop()
{        
    if (m_processMode == Process_none)
        return;
    
    m_process.setProperty("userKill", 1);
    m_process.kill();
    m_process.waitForFinished();    
    updateActionStatus();
}

void MainWindow::onActionBreakPoint()
{
    if(m_editor && m_editor->type() == Editor_script)
    {
        JZNodeEditor *node_editor = (JZNodeEditor*)m_editor;
        node_editor->breakPointTrigger();        
        m_breakPoint->updateBreakPoint();
    }
}

void MainWindow::onActionStepOver()
{
    m_debuger.stepOver();
    updateActionStatus();
}

void MainWindow::onActionStepIn()
{
    m_debuger.stepIn();
    updateActionStatus();
}

void MainWindow::onActionStepOut()
{
    m_debuger.stepOut();
    updateActionStatus();
}

void MainWindow::onActionModbus()
{
    if (!m_simulator)
        m_simulator = new JZModbusSimulator(this);

    m_simulator->showNormal();
}

void MainWindow::onActionHelp()
{
    onActionAbout();
}

void MainWindow::onActionCheckUpdate()
{
    JZUpdateClient client(qApp->applicationDirPath());
    if (!client.init("120.77.183.99", 8888))
    {
        QMessageBox::information(this, "", "连接服务器失败");
        return;
    }

    bool update = false;
    if (!client.checkUpdate(update))
    {
        QMessageBox::information(this, "", "获取更新信息失败");
        return;
    }

    if (update)
    {
        QMessageBox::information(this, "", "当前已经是最新版本");
        return;
    }
    else
    {
        if (QMessageBox::question(this, "", "发现新版本，是否更新", QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;

        if (!closeProject())
            return;
        
        JZUpdateDialog dlg(this);
        dlg.setClient(&client);
        if (dlg.exec() != QDialog::Accepted)        
            return;        
        
        QString exe_path = qApp->applicationFilePath();        
        QProcess::startDetached(exe_path);
        close();         
    }
}

void MainWindow::onActionAbout()
{
    JZAboutDialog dlg(this);
    dlg.exec();    
}

void MainWindow::onModifyChanged(bool flag)
{        
    auto editor = qobject_cast<JZEditor*>(sender());
    int index = m_editorStack->indexOf(editor);
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

bool MainWindow::closeProject()
{
    if (!closeAllEditor())
        return false;
    
    m_projectTree->clear();
    m_breakPoint->clear();
    m_project.close();
    updateActionStatus();
    setWindowTitle("JZNodeEditor");    
    return true;
}

bool MainWindow::openProject(QString filepath)
{    
    if (!m_project.open(filepath))
    {
        QMessageBox::information(this, "", "打开工程失败: " + m_project.error());
        return false;
    }
    
    m_buildInfo.clear();
    m_projectTree->setProject(&m_project);
    m_setting.addRecentProject(m_project.filePath());
    m_breakPoint->updateBreakPoint();     
    updateActionStatus();
    setWindowTitle(m_project.name());
    return true;
}

JZEditor *MainWindow::createEditor(int type)
{
    JZEditor *editor = nullptr;
    if(type == ProjectItem_scriptParamBinding || type == ProjectItem_scriptFunction)
        editor = new JZNodeEditor();
    else if(type == ProjectItem_param)
        editor = new JZNodeParamEditor();
    else if(type == ProjectItem_ui)
        editor = new JZUiEditor();

    if(editor)
        editor->setProject(&m_project);
    return editor;
}

void MainWindow::gotoNode(QString file, int nodeId)
{
    if (openEditor(file))
    {
        JZNodeEditor *editor = qobject_cast<JZNodeEditor*>(m_editor);
        editor->ensureNodeVisible(nodeId);
    }
}

void MainWindow::onFunctionOpen(QString functionName)
{
    auto file = m_project.functionItem(functionName);
    openEditor(file->itemPath());
}

void MainWindow::onAutoCompiler()
{
    m_buildInfo.changeTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void MainWindow::onAutoRun()
{
    auto edit = qobject_cast<JZNodeEditor*>(sender());
    if(m_editor != edit)
        return;
    
    if(m_processMode == Process_none)
    {
        if(m_buildInfo.changeTimestamp >= m_buildInfo.buildTimestamp)
        {
            m_buildInfo.runItemPath = edit->script()->itemPath();
            build();
        }
        else
        {
            startUnitTest(edit->script()->itemPath());
        }
    }
}

void MainWindow::showTopLevel()
{   
    if(isActiveWindow())
        return;
        
    Qt::WindowFlags flags = windowFlags();
    this->setWindowFlags((flags | Qt::WindowStaysOnTopHint));
    this->show();

    this->setWindowFlags(flags);
    this->show();
    raise();
    activateWindow();
}

void MainWindow::onAutoCompilerTimer()
{   
    qint64 cur = QDateTime::currentMSecsSinceEpoch();    
    if (cur - m_buildInfo.changeTimestamp <= 1000)
        return;

    if (m_project.isNull())
        return;

    if(m_buildInfo.changeTimestamp >= m_buildInfo.buildTimestamp)
        build();
}

void MainWindow::onBuildFinish(bool flag)
{
    QString result = flag ? "successed" : "failed";
    m_log->addLog(Log_Compiler, "build finish:" + result);

    auto builder = m_buildThread.builder();
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        if (it.value()->type() == Editor_script)
        {
            auto node_edit = (JZNodeEditor*)it.value();
            auto cmp_info = builder->compilerInfo(node_edit->script());
            if(cmp_info)
                node_edit->setCompilerResult(cmp_info);
        }        
        it++;
    }
    
    m_buildInfo.success = flag;
    if(m_buildInfo.success)
    {
        if(m_buildInfo.isUnitTest())
        {
            startUnitTest(m_buildInfo.runItemPath);
        }
        if(m_buildInfo.save)
        {
            saveProgram();
            m_buildInfo.saveTimestamp = QDateTime::currentMSecsSinceEpoch();
        }
        if(m_buildInfo.start)
            startProgram();
    }
    m_buildInfo.clearTask();
}

void MainWindow::onAutoRunResult(UnitTestResultPtr result)
{
    if(result->result == UnitTestResult::Cancel)
        return;

    auto script_item = m_project.functionItem(result->function);
    if(!script_item)
        return;
    
    JZEditor *e = editor(script_item->itemPath());
    if(!e)
        return;

    JZNodeEditor *node_e = qobject_cast<JZNodeEditor*>(e);
    node_e->setAutoRunResult(*result);
}   

JZEditor *MainWindow::editor(QString filepath)
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

QList<JZNodeEditor*> MainWindow::nodeEditorList()
{
    QList<JZNodeEditor*> list;
    //editor
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (it.value()->type() == Editor_script)
        {
            auto node_edit = (JZNodeEditor*)it.value();
            list << node_edit;
        }
        it++;
    }
    return list;
}

JZNodeEditor *MainWindow::nodeEditor(QString filepath)
{
    JZEditor *e = editor(filepath);
    if(!e)
        return nullptr;

    return qobject_cast<JZNodeEditor*>(e);
}

void MainWindow::switchEditor(JZEditor *editor)
{
    if(m_editor)
        m_editor->removeMenuBar(this->menuBar());

    m_editor = editor;
    if(editor != nullptr)
    {        
        QMenu *menu;        

        m_editorStack->setCurrentWidget(m_editor);
        m_editor->addMenuBar(this->menuBar());
        m_editor->active();
        m_editor->setFocus();
    }
    else
        m_editorStack->setCurrentIndex(0);
    updateActionStatus();
}

bool MainWindow::openEditor(QString filepath)
{
    if (filepath == "__idle__")
        return false;

    JZProjectItem *item = m_project.getItem(filepath);
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
        if (new_edit->type() == Editor_script)
        {
            auto node_edit = (JZNodeEditor*)new_edit;
            connect(node_edit, &JZNodeEditor::sigFunctionOpen, this, &MainWindow::onFunctionOpen);
            connect(node_edit, &JZNodeEditor::sigAutoCompiler, this, &MainWindow::onAutoCompiler);
            connect(node_edit, &JZNodeEditor::sigAutoRun, this, &MainWindow::onAutoRun);
            connect(node_edit, &JZNodeEditor::sigRuntimeValueChanged, this, &MainWindow::onEditorValueChanged);

            node_edit->setRunningMode(m_processMode);
            auto cmp_ret = compilerResult(file);
            if(cmp_ret)
                node_edit->setCompilerResult(cmp_ret);
        }

        m_editors[item] = new_edit;
        m_editorStack->addTab(new_edit, filepath);
    }
    switchEditor(new_edit);
    
    return true;
}

void MainWindow::resetEditor(JZEditor *editor)
{
    if (editor->type() == Editor_script)
    {
        auto node_edit = (JZNodeEditor*)editor;
        node_edit->resetFile();
    }        
}

void MainWindow::closeEditor(JZEditor *editor)
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
            resetEditor(editor);            
        }
        else if (ret == QMessageBox::Cancel)
        {            
            return;
        }
    }    
    editor->close();
    editor->setItem(nullptr);
    
    m_editors.remove(item);
    if(m_editor == editor)
    {
        if(m_editors.size() > 0)
            switchEditor(m_editors.first());
        else
            switchEditor(nullptr);
    }

    int index = m_editorStack->indexOf(editor);
    m_editorStack->removeTab(index);
    delete editor;
}

void MainWindow::openItem(QString filepath)
{
    openEditor(filepath);
}

void MainWindow::closeItem(QString filepath)
{
    auto edit = editor(filepath);
    if(!edit)
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

void MainWindow::onProjectTreeAction(int type, QString filepah)
{
    if (type == Action_open)
        openItem(filepah);
    else if (type == Action_close)
        closeItem(filepah);
    else if (type == Action_remove)
        removeItem(filepah);
}

void MainWindow::onEditorClose(int index)
{
    JZEditor *editor = qobject_cast<JZEditor*>(m_editorStack->widget(index));
    closeEditor(editor);
}

void MainWindow::onEditorActivite(int index)
{
    if (index == -1)
        return;

    JZEditor *editor = qobject_cast<JZEditor*>(m_editorStack->widget(index));
    editor->active();
    switchEditor(editor);
}

void MainWindow::onNavigate(QUrl url)
{
    if(openEditor(url.path()))
    {
        m_editor->navigate(url);
    }
}

void MainWindow::onProjectChanged()
{   
    m_buildInfo.changeTimestamp = QDateTime::currentMSecsSinceEpoch();

    //editor
    auto list = nodeEditorList();
    for(auto node_edit : list)
        node_edit->updateDefine();        
}

void MainWindow::onProjectItemChanged(JZProjectItem *item)
{
    //editor
    if (m_editors.contains(item))
    {
        int index = m_editorStack->indexOf(m_editors[item]);
        if (m_editorStack->tabText(index) != item->itemPath())
            updateTabText(index);
    }
    onProjectChanged();
}

void MainWindow::onStackChanged(int stack_index)
{
    updateRuntime(stack_index, false);
}

void MainWindow::onEditorValueChanged(int id,QString value)
{
    onWatchValueChanged(irId(id),value);
}

void MainWindow::onWatchValueChanged(JZNodeIRParam coor, QString value)
{    
    JZNodeSetDebugParam param_info;
    param_info.stack = m_stack->stackIndex();
    param_info.coor = coor;
    param_info.value = value;
    
    JZNodeSetDebugParamResp result;
    if(!m_debuger.setVariable(param_info,result))
        return;

    JZNodeGetDebugParamResp get_resp;
    get_resp.stack = result.stack;
    get_resp.coors << result.coor;
    get_resp.values << result.value;
    
    for(auto w : m_debugWidgets)
        w->updateParamInfo(&get_resp);
    if (coor.isStack())
    {
        auto stack_info = m_runtime.stacks[param_info.stack];
        auto gemo = JZNodeCompiler::paramGemo(coor.id());
        setRuntimeValue(stack_info.file,gemo.nodeId,gemo.pinId,result.value);
    }
}

void MainWindow::onWatchNameChanged(JZNodeIRParam coor)
{    
    JZNodeGetDebugParam param_info;
    param_info.stack = m_stack->stackIndex();
    param_info.coors << coor;
    
    JZNodeGetDebugParamResp ret; 
    if(!m_debuger.getVariable(param_info,ret))
        return;

    m_watchManual->updateParamInfo(&ret);        
}

void MainWindow::onWatchNotify()
{
    auto inst = JZNodeParamDelegateManager::instance();
    if(m_runThread.engine()->stack()->size() != 1)
        return;

    QString file = m_runThread.engine()->stack()->currentEnv()->script->file;
    auto e = nodeEditor(file);
    if(!e || e != m_editor)
        return;

    auto &watchMap = m_runThread.engine()->stack()->currentEnv()->watchMap;
    auto it = watchMap.begin();
    while(it != watchMap.end())
    {
        JZNodeGemo gemo = JZNodeCompiler::paramGemo(it.key());
        JZNodeDebugParamValue value;
        value.type = JZNodeType::variantType(it.value());
        value.value = JZNodeType::debugString(it.value());

        auto data_type = JZNodeType::variantType(it.value());
        auto d = inst->delegate(data_type);
        if (d && d->pack)
            value.binValue = d->pack(it.value());

        e->setRuntimeValue(gemo.nodeId,gemo.pinId,value);
        it++;
    }
}

void MainWindow::onRuntimeWatch(const JZNodeRuntimeWatch &info)
{
    QString file = info.runtimInfo.stacks.back().file;
    auto edit = nodeEditor(file);
    if (!edit)
        return;

    auto it = info.values.begin();
    while (it != info.values.end())
    {
        auto gemo = JZNodeCompiler::paramGemo(it.key());
        setRuntimeValue(file,gemo.nodeId,gemo.pinId,it.value());
        it++;
    }    
}

void MainWindow::updateAutoWatch(int stack_index)
{
    if (m_program.isNull())
        return;

    auto &stack = m_runtime.stacks[stack_index];

    JZNodeGetDebugParam param_info;
    param_info.stack = stack_index;
    
    auto func = m_program.function(stack.function);
    if(func->isCFunction)
    {
        m_watchAuto->clear();
        return;
    }

    auto func_debug = m_program.script(stack.file)->functionDebug(stack.function);
    if (func->isMemberFunction())
    {        
        param_info.coors << irThis();
    }

    //local    
    for (int i = 0; i < func_debug->localVariables.size(); i++)
    {
        auto &local = func_debug->localVariables[i];        
        param_info.coors << irRef(local.name);
    }

    int node_prop_index = param_info.coors.size();
    const auto &node_info = func_debug->nodeInfo[stack.nodeId];
    for (int i = 0; i < node_info.paramIn.size(); i++)
    {
        int param_id = JZNodeCompiler::paramId(node_info.id,node_info.paramIn[i].id);
        param_info.coors << irId(param_id);
    }
    for (int i = 0; i < node_info.paramOut.size(); i++)
    {
        int param_id = JZNodeCompiler::paramId(node_info.id,node_info.paramOut[i].id);
        param_info.coors << irId(param_id);
    }

    JZNodeGetDebugParamResp param_info_resp;
    if(!m_debuger.getVariable(param_info,param_info_resp))
        return;

    m_watchAuto->setParamInfo(&param_info_resp);

    auto edit = nodeEditor(stack.file);
    if (edit)
    {
        for (int i = node_prop_index; i < param_info_resp.coors.size(); i++)
        {
            auto &coor = param_info_resp.coors[i];
            if(coor.isStack())
            {
                auto gemo = JZNodeCompiler::paramGemo(coor.id());
                edit->setRuntimeValue(gemo.nodeId,gemo.pinId,param_info_resp.values[i]);
            }
        }
    }
}

void MainWindow::setRuntimeValue(QString file,int node_id,int pin_id,const JZNodeDebugParamValue &value)
{
    auto editor = nodeEditor(file);
    if(!editor)
        return;

    editor->setRuntimeValue(node_id,pin_id,value);
}

void MainWindow::updateRuntime(int stack_index,bool isNew)
{
    if (stack_index == -1)
    {
        clearWatchs();
        return;
    }

    auto stack = m_runtime.stacks[stack_index];
    if (stack.file == "__idle__")
    {
        clearWatchs();
        return;
    }                
    
    //this
    for (int i = stack_index; i >= 0; i--)
    {
        auto &top = m_runtime.stacks[i];
        if (!top.file.isEmpty())
        {
            setRuntimeNode(top.file, top.nodeId);
            break;
        }
    }
    if (stack.file.isEmpty()) //in c function
        return;    

    //watch auto
    updateAutoWatch(stack_index);

    //watch manual    
    JZNodeGetDebugParam param_info_watch;
    param_info_watch.stack = stack_index;

    QStringList watch_list = m_watchManual->watchList();
    for (int i = 0; i < watch_list.size(); i++)
    {        
        param_info_watch.coors << irRef(watch_list[i]);
    }

    JZNodeGetDebugParamResp param_info_watch_resp;
    if(!m_debuger.getVariable(param_info_watch,param_info_watch_resp))
        return;
        
    m_watchManual->setParamInfo(&param_info_watch_resp); 
}

void MainWindow::build()
{    
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    m_log->clearLog(Log_Compiler);
    m_log->addLog(Log_Compiler, "[" + time + "] ===== start build =====");    
    
    m_runThread.stopRun();
    m_buildThread.stopBuild();
    m_buildThread.startBuild();
    m_buildInfo.buildTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void MainWindow::saveToFile(QString filepath,QString text)
{
    QFile file(filepath);
    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream s(&file);
        s.setCodec("utf-8");
        s << text;
        file.close();
    }
}

void MainWindow::saveProgram()
{
    QString build_path = m_project.path() + "/build";
    QString build_exe = build_path + "/" + m_project.name() + ".program";

    QElapsedTimer timer;
    timer.start();

    QDir dir;
    if (!dir.exists(build_path))
        dir.mkdir(build_path);
    if (!m_program.save(build_exe))
    {
        m_log->addLog(Log_Compiler, "generate program failed");
        return;
    }
    saveToFile(build_path + "/" + m_project.name() + ".jsm", m_program.dump());
}

void MainWindow::startUnitTest(QString runItemPath)
{
    qDebug() << "start unit test" << runItemPath;

    auto e = nodeEditor(runItemPath);
    if(e)
        m_runThread.startRun(&m_program,e->scriptTestDepend());
}

void MainWindow::startProgram()
{    
    qDebug() << "startProgram";
    
    m_runThread.stopRun();

    m_log->clearLog(Log_Runtime);   
    QString app = qApp->applicationFilePath();
    QString build_exe = m_project.path() + "/build/" + m_project.name() + ".program";
    QStringList params;
    params << "--run" << build_exe << "--debug";

    m_log->addLog(Log_Runtime, "start program");
    m_process.setWorkingDirectory(m_project.path());
    m_process.start(app, params);
    if (!m_process.waitForStarted())
    {
        QMessageBox::information(this, "", "start failed");
        return;
    }

    QThread::msleep(100);
    if(!m_debuger.connectToServer("127.0.0.1",19888))
    {
        QMessageBox::information(this,"","can't connect to process");
        return;
    }
    m_log->addLog(Log_Runtime, "conenct to server");

    JZNodeDebugInfo info;
    info.breakPoints = m_project.breakPoints();
    JZNodeProgramInfo program_info; 
    if(!m_debuger.init(info,program_info))
    {
        m_log->addLog(Log_Runtime, "load debug info failed.");
    }
    if(!m_program.load(program_info.appPath))
    {
        m_log->addLog(Log_Runtime, "load debug info failed.");
    }

    setRunningMode(Process_running);
}

void MainWindow::onTabContextMenu(QPoint pos)
{
    QMenu menu(this);
    QAction *actSave = menu.addAction("保存");
    QAction *actClose = menu.addAction("关闭");
    QAction *actAll = menu.addAction("关闭所有文档");
    QAction *actAllExcept = menu.addAction("除此之外全部关闭");

    auto bar = qobject_cast<QTabBar*>(sender());
    QAction *ret = menu.exec(bar->mapToGlobal(pos));
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
        auto editor = qobject_cast<JZEditor*>(m_editorStack->widget(index));
        closeAllEditor(editor);
    }
}

void MainWindow::onLog(LogObjectPtr log)
{
    m_log->addLog(log->module, log->message);
}

void MainWindow::onRuntimeStatus(int status)
{        
    ProcessStatus process_status;
    if (status == Status_idlePause || status == Status_pause)
        process_status = Process_pause;
    else    
        process_status = Process_running;    

    setRunningMode(process_status);    
}

void MainWindow::onRuntimeLog(QString log)
{
    m_log->addLog(Log_Runtime, log);
}

void MainWindow::onRuntimeError(JZNodeRuntimeError error)
{
    QString error_msg = "Runtime Error: " + error.error + "\n\n";
    
    int stack_size = error.info.stacks.size();
    for (int i = 0; i < stack_size; i++)
    {
        auto s = error.info.stacks[stack_size - i - 1];
        QString line = makeLink(s.file, s.function, "id=" + QString::number(s.nodeId));
        m_log->addLog(Log_Runtime, line);
        
        line = s.function;
        if (!s.file.isEmpty())
            line += +"(" + s.file + "," + QString::number(s.nodeId) + ")";        
        error_msg += line + "\n";
    }    

    m_log->addLog(Log_Runtime, error_msg);
    showTopLevel();    
    QMessageBox::information(this, "", error_msg);    
}

void MainWindow::onNetError()
{
    m_log->addLog(Log_Runtime, "调试连接中断");
    onActionStop();
}

void MainWindow::onRuntimeFinish(int code,QProcess::ExitStatus status)
{    
    if (status == QProcess::CrashExit)
    {
        if (m_process.property("userKill").isValid())
            m_process.setProperty("userKill", QVariant());
        else
            m_log->addLog(Log_Runtime, "process crash ");
    }
    else
        m_log->addLog(Log_Runtime, "process finish, exit code " + QString::number(code));

    setRunningMode(Process_none);
    updateActionStatus();
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

bool MainWindow::closeAllEditor(JZEditor *except)
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
                    resetEditor(editor);
                    if(ret == QMessageBox::NoToAll)
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
                resetEditor(editor);
            }
        }
        editor->close();
        close_list << editor;        
        
        it++;
    }
    m_project.saveCommit();

    for (auto editor : close_list)
    {
        int index = m_editorStack->indexOf(editor);
        m_editorStack->removeTab(index);
        m_editors.remove(editor->item());
        delete editor;
    }            
    
    m_editor = nullptr;
    switchEditor(except);    
    return true;
}

void MainWindow::setRunningMode(ProcessStatus flag)
{
    if (flag == m_processMode)
        return;

    m_processMode = flag;
    setWatchStatus(flag);
    m_stack->setRunningMode(flag);

    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        if (it.value()->type() == Editor_script)
        {
            auto node_edit = (JZNodeEditor*)it.value();
            node_edit->setRunningMode(flag);
        }        
        it++;
    }

    if (m_processMode == Process_none)
    {
        m_program.clear();
        m_runtime = JZNodeRuntimeInfo();
    }

    //update
    if (m_processMode == Process_pause)
    {
        JZNodeRuntimeInfo new_runtime;
        if (!m_debuger.runtimeInfo(new_runtime))
        {
            m_log->addLog(Log_Runtime, "获取信息失败");
            return;
        }

        bool isNew = true;
        if (new_runtime.stacks.size() > 0 && new_runtime.stacks.size() == m_runtime.stacks.size()
            && new_runtime.stacks.back().file == m_runtime.stacks.back().file
            && new_runtime.stacks.back().function == m_runtime.stacks.back().function)
        {
            isNew = false;
        }

        showTopLevel();

        m_runtime = new_runtime;
        m_log->stack()->setRuntime(m_runtime);
        updateRuntime(m_runtime.stacks.size() - 1, isNew);
    }
    else
    {                
        clearRuntimeNode();
    }    
    updateActionStatus();
}

void MainWindow::clearRuntimeNode()
{
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (editor->type() == Editor_script)
            ((JZNodeEditor*)editor)->setRuntimeNode(-1);
        it++;
    }
}

void MainWindow::setRuntimeNode(QString file, int nodeId)
{
    clearRuntimeNode();
    if (openEditor(file))
    {
        JZNodeEditor *editor = qobject_cast<JZNodeEditor*>(m_editor);
        editor->setRuntimeNode(nodeId);
    }
}

void MainWindow::clearWatchs()
{
    for (auto w : m_debugWidgets)
        w->clear();
}

void MainWindow::setWatchStatus(ProcessStatus status)
{
    for (auto w : m_debugWidgets)
        w->setRunningMode(status);
}

void MainWindow::onBreakPointClicked(QString file, int id)
{
    onNavigate(file + "?id=" + QString::number(id));
}

void MainWindow::onBreakPointChanged(BreakPointChange reason, QString file, int id)
{    
    if (m_debuger.isConnect())
    {
        if (reason == BreakPoint_add)
        {
            auto bt = m_project.breakPoint(file, id);
            m_debuger.addBreakPoint(bt);
        } 
        else if(reason == BreakPoint_remove)
            m_debuger.removeBreakPoint(file, id);
    }
}

void MainWindow::updateTabText(int index)
{
    auto editor = qobject_cast<JZEditor*>(m_editorStack->widget(index));
    QString title = editor->item()->itemPath();
    if (editor->isModified())
        title += "*";
    m_editorStack->setTabText(index, title);
}