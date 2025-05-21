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
#include "JZAboutDialog.h"
#include "JZProjectSettingDialog.h"
#include "JZNodeProgramDumper.h"
#include "JZProjectTemplate.h"
#include "JZNodeEditorManager.h"
#include "JZEditorUtils.h"
#include "JZNodeUtils.h"
#include "LogManager.h"
#include "JZNodeLangServer.h"
#include "modules/communication/modbus/JZModbusSimulator.h"

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

//ActionStatus
MainWindow::ActionStatus::ActionStatus(QAction *act, QVector<int> act_flags)
{
    this->action = act;
    this->flags = act_flags;
}

//MainWindow
MainWindow *g_mainWindow = nullptr;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    g_mainWindow = this;
    m_editor = nullptr;    
    m_processMode = Process_none;    

    JZNodeEditorInit();
    
    LogManagerInit();
    JZLogManager::instance()->addObserver(Log_Compiler,this);
    JZLogManager::instance()->addObserver(Log_Runtime, this);

    auto lang_inst = JZNodeLangServer::instance();
    lang_inst->setProject(&m_project);

    connect(&m_debuger,&JZNodeDebugClient::sigLog,this,&MainWindow::onRuntimeLog, Qt::QueuedConnection);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeError,this,&MainWindow::onRuntimeError, Qt::QueuedConnection);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeStatus, this, &MainWindow::onRuntimeStatus, Qt::QueuedConnection);    
    connect(&m_debuger,&JZNodeDebugClient::sigNetError, this, &MainWindow::onNetError, Qt::QueuedConnection);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeWatch, this, &MainWindow::onRuntimeWatch, Qt::QueuedConnection);

    connect(&m_process,(void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,this,&MainWindow::onRuntimeFinish);
    connect(&m_project,&JZProject::sigItemChanged, this, &MainWindow::onProjectItemChanged);
    connect(&m_project,&JZProject::sigDefineChanged, this, &MainWindow::onProjectChanged);
    connect(&m_project,&JZProject::sigBreakPointChanged, this, &MainWindow::onBreakPointChanged);

    m_task.setProject(&m_project);    
    connect(m_task.runThread(),&JZNodeAutoRunThread::sigResult,this, &MainWindow::onAutoRunResult);
    connect(&m_task, &MainTaskManager::sigTaskRunning, this, &MainWindow::onTaskRunning);
    connect(&m_task, &MainTaskManager::sigBuildStart, this, &MainWindow::onBuildStart);
    connect(&m_task, &MainTaskManager::sigBuildFinish, this, &MainWindow::onBuildFinish);

    auto engine = m_task.runThread()->engine();
    connect(engine,&JZNodeEngine::sigWatchNotify,this,&MainWindow::onWatchNotify,Qt::BlockingQueuedConnection);
    m_task.runThread()->unitTest()->setProject(&m_project);

    loadSetting();    
    initUi();     
    updateActionStatus();    

    m_config.init(qApp->applicationDirPath() + "/config.db");
    m_config.addConfig("JZModbusSimulatorConfig");
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

    actUndo->setShortcutContext(Qt::WidgetShortcut);
    actRedo->setShortcutContext(Qt::WidgetShortcut);
    actDel->setShortcutContext(Qt::WidgetShortcut);
    actCut->setShortcutContext(Qt::WidgetShortcut);
    actCopy->setShortcutContext(Qt::WidgetShortcut);
    actPaste->setShortcutContext(Qt::WidgetShortcut);
    actSelectAll->setShortcutContext(Qt::WidgetShortcut);

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
    auto menu_export = menu_build->addMenu("导出");
    auto actExportExe = menu_export->addAction("导出Exe");
    auto actExportCpp = menu_export->addAction("导出Cpp");
    connect(actBuild,&QAction::triggered,this,&MainWindow::onActionBuild);
    connect(actExportExe, &QAction::triggered, this, &MainWindow::onActionExportExe);
    connect(actExportCpp,&QAction::triggered,this,&MainWindow::onActionExportCpp);
    m_actionStatus << ActionStatus(actBuild, { as::ProjectVaild, as::ProcessIsEmpty });

    QMenu *menu_tool = menubar->addMenu("工具");
    auto actModbus = menu_tool->addAction("Modbus");
    menu_tool->addAction("性能分析");

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

    menu_debug->addSeparator();
    auto actDebugSetting = menu_debug->addAction("选项");

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
    connect(actDebugSetting, &QAction::triggered, this, &MainWindow::onActionDebugSetting);

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

    connect(actModbus, &QAction::triggered, this, &MainWindow::onActionModbus);

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

    m_watch = m_log->watch();       
    connect(m_watch, &JZNodeWatch::sigSetWatch, this, &MainWindow::onSetWatch);
    connect(m_watch, &JZNodeWatch::sigGetWatch, this, &MainWindow::onGetWatch);
    
    m_breakPoint = m_log->breakpoint();    
    m_breakPoint->setProject(&m_project);
    connect(m_breakPoint, &JZNodeBreakPointWidget::sigBreakPointClicked, this, &MainWindow::onBreakPointClicked);    

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

void MainWindow::customEvent(QEvent *event)
{
    if(event->type() == JZLogEvent::EventType)    
    {
        auto log_event = dynamic_cast<JZLogEvent*>(event);
        auto log = log_event->log;
        m_log->addLog(log->module, log->message);
    }
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

    for (int i = 0; i < m_floatWidgets.size(); i++)
        m_floatWidgets[i]->close();
    m_floatWidgets.clear();

    m_task.clearTask();
    m_task.stopRunThread();    
    QMainWindow::closeEvent(event);
}

const CompilerResult *MainWindow::compilerResult(const QString &path)
{
    if (!m_buildResult)
        return nullptr;

    auto it = m_buildResult->compilerResult.find(path);
    if (it == m_buildResult->compilerResult.end())
        return nullptr;

    return &it.value();
}

void MainWindow::updateAutoRunDepend()
{
    auto list = nodeEditorList();
    for (int i = 0; i < list.size(); i++)
        list[i]->setDepend(nullptr);

    auto node_editor = currentNodeEditor();
    if (!node_editor)
        return;

    if (m_buildResult->status == Build_Successed)
    {
        auto test = m_task.runThread()->unitTest();
        test->setScript(node_editor->script());
        node_editor->setDepend(test->depend());
    }
}

JZProject *MainWindow::project()
{
    return &m_project;
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
    
    m_toolDebug->setVisible(isProcess);
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
    
    QString project_path = project_dir + "/" + name + ".jzproj";
    QString project_tmp = dialog.projectType();        
    JZProjectTemplate::instance()->initProject(&m_project, project_tmp);
    JZEditorUtils::projectUpdateLayout(&m_project);
    if (!m_project.saveAs(project_path) || !m_project.saveAllItem())
    {
        QMessageBox::information(this, "", "新建工程失败");
        return;
    }
    openProject(project_path);    
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
        m_editor->selectAll();
}

void MainWindow::onActionProjectProp()
{
    JZProjectSettingDialog dlg(this);
    dlg.setProject(&m_project);
    dlg.exec();
}

void MainWindow::onActionBuild()
{
    if (m_processMode != Process_none)
    {
        if (QMessageBox::question(this, "", "是否停止调试", QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            return;

        onActionStop();
    }

    saveAll();
    m_task.addBuildTask();
}

void MainWindow::onActionRun()
{
    saveAll();
    m_task.addRunningTask();
}

void MainWindow::onActionExportExe()
{
    if (m_project.isNull())
        return;

    m_task.addExportExeTask();
}

void MainWindow::onActionExportCpp()
{
    if (m_project.isNull())
        return;

    m_task.addExportCppTask();    
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
    stopProgram();    
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

void MainWindow::onActionDebugSetting()
{
    JZDebugSettingDialog dlg(this);
    dlg.setConfig(m_debugSettting);
    if (dlg.exec() != JZDebugSettingDialog::Accepted)
        return;

    m_debugSettting = dlg.config();
}

void MainWindow::onActionModbus()
{
    JZModbusSimulator *simulator = new JZModbusSimulator();
    connect(simulator, &JZModbusSimulator::sigClose, this, &MainWindow::onModbusSimulatorClose);

    JZModbusSimulatorConfig cfg = m_config.getConfig<JZModbusSimulatorConfig>("JZModbusSimulatorConfig");
    simulator->setConfig(cfg);
    simulator->show();
    m_floatWidgets << simulator;
}

void MainWindow::onModbusSimulatorClose()
{
    JZModbusSimulator *simulator = qobject_cast<JZModbusSimulator*>(sender());
    auto cfg = simulator->config();
    m_config.setConfig<JZModbusSimulatorConfig>("JZModbusSimulatorConfig",cfg);
    m_floatWidgets.removeAll(simulator);
}

void MainWindow::onActionHelp()
{
    onActionAbout();
}

void MainWindow::onActionCheckUpdate()
{
    QMessageBox::information(this, "", "还没实现");
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
    
    m_log->clearLogs();    
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
    if(type == ProjectItem_scriptItem)
        editor = new JZNodeEditor();
    else if(type == ProjectItem_param)
        editor = new JZNodeParamEditor();
    else if(type == ProjectItem_ui)
        editor = new JZUiEditor();

    if (editor)
    {
        editor->setMainWindow(this);
        editor->setProject(&m_project);        
    }
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
    m_task.addAutoCompilerTask();
}

void MainWindow::onAutoRunOnce()
{

}

void MainWindow::onAutoRun()
{
    auto edit = qobject_cast<JZNodeEditor*>(sender());
    if(m_editor != edit)
        return;
    
    startUnitTest(edit->script()->itemPath());    
}

void MainWindow::onAutoRunStop()
{
    stopUnitTest();
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

void MainWindow::onBuildStart()
{
    m_log->clearLog(Log_Compiler);    
}

void MainWindow::onBuildFinish(JZNodeBuildResultPtr result)
{
    m_buildResult = result;
        
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        if (it.value()->type() == Editor_script)
        {
            auto node_edit = (JZNodeEditor*)it.value();
            auto cmp_info = compilerResult(it.key()->itemPath());
            if (cmp_info)
                node_edit->setCompilerResult(cmp_info);
        }
        it++;
    }
    updateAutoRunDepend();
}

void MainWindow::onAutoRunResult(int result)
{
    auto engine = m_task.runThread()->engine();    
}   

void MainWindow::onTaskRunning()
{
    QString prog_path = m_project.path() + "/build/" + m_project.name() + ".program";
    QString error;
    if (!m_program.load(prog_path, error))
    {
        m_log->addLog(Log_Runtime, "load program failed. " + error);
        return;
    }

    startProgram();    
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

JZNodeEditor *MainWindow::currentNodeEditor()
{
    if (m_editor && m_editor->type() == Editor_script)
        return dynamic_cast<JZNodeEditor*>(m_editor);
    else
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
    if (editor == m_editor)
        return;

    if (m_editor)    
        m_editor->inactive‌();    

    m_editor = editor;    
    if(editor != nullptr)
    {                
        m_editorStack->setCurrentWidget(m_editor);
        m_editor->active();
        m_editor->setFocus();

        updateAutoRunDepend();
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
            connect(node_edit, &JZNodeEditor::sigAutoRunOnce, this, &MainWindow::onAutoRunOnce);
            connect(node_edit, &JZNodeEditor::sigAutoRun, this, &MainWindow::onAutoRun);
            connect(node_edit, &JZNodeEditor::sigAutoRunStop, this, &MainWindow::onAutoRunStop);
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
    switchEditor(editor);
}

void MainWindow::onNavigate(QUrl url)
{
    QString path = url.path();
    if(openEditor(path))
    {
        m_editor->navigate(url);
    }
}

void MainWindow::onProjectChanged()
{   
    m_task.addAutoCompilerTask();

    //editor
    auto list = nodeEditorList();
    for(auto node_edit : list)
        node_edit->updateDefine();        
}

void MainWindow::onProjectItemChanged(JZProjectItem *item)
{    
    //editor
    auto it = m_editors.begin();
    while(it != m_editors.end())
    {
        int index = m_editorStack->indexOf(it.value());                
        updateTabText(index);

        it++;
    }
    onProjectChanged();
}

void MainWindow::onStackChanged(int stack_index)
{
    updateRuntime(stack_index, false);
}

void MainWindow::onEditorValueChanged(int id,QString value)
{
    onSetWatch(irId(id),value);
}

void MainWindow::onSetWatch(JZNodeIRParam coor, QString value)
{    
    JZNodeSetDebugParam param_info;
    param_info.stack = m_stack->stackIndex();
    param_info.coor = coor;
    param_info.value = value;
    
    JZNodeSetDebugParamResp result;
    if (!m_debuger.setVariable(param_info, result))
    {        
        onGetWatch(coor);  //设置失败重新刷新下值
        return;
    }
    
    JZNodeGetDebugParamResp get_resp;
    m_watch->updateParamInfo(&get_resp);
    if (coor.isStack())
    {
        auto stack_info = m_runtime.stacks[param_info.stack];
        auto gemo = JZNodeCompiler::paramGemo(coor.id());
        setRuntimeValue(stack_info.scriptItemPath, gemo.nodeId,gemo.pinId, get_resp.values[0]);
    }
}

void MainWindow::onGetWatch(JZNodeIRParam coor)
{    
    auto cur_stack = currentStack();

    JZNodeGetDebugParam param_info;
    param_info.stack = m_stack->stackIndex();
    param_info.coors << coor;
    
    JZNodeGetDebugParamResp ret; 
    if(!m_debuger.getVariable(param_info,ret))
        return;

    m_watch->updateParamInfo(&ret);
    if (coor.isStack())
    {
        JZNodeGemo gemo = JZNodeGemo::fromParamId(coor.id());
        setRuntimeValue(cur_stack->scriptItemPath, gemo.nodeId, gemo.pinId, ret.values[0]);
    }
}

JZNodeRuntimeInfo::Stack *MainWindow::currentStack()
{
    int index = m_stack->stackIndex();
    return &m_runtime.stacks[index];
}

void MainWindow::onWatchNotify()
{
    JZNodeEngine *engine = m_task.runThread()->engine();
    if(m_editor->type() != Editor_script)
        return;

    JZNodeEditor *e = qobject_cast<JZNodeEditor*>(m_editor);    
    QString file = engine->stack()->currentEnv()->script->itemPath;
    if (file != e->item()->itemPath())
        return;
    
    auto param_env = engine->stack()->currentEnv();
    auto watchList = e->view()->watchList();

    for(int i = 0; i < watchList.size(); i++)
    {
        int param_id = watchList[i];
        QVariantPtr *ref = param_env->getRef(param_id);
        auto gemo = JZNodeGemo::fromParamId(param_id);
        e->view()->displayValue(gemo.nodeId, gemo.pinId, ref);
    }
}

void MainWindow::onRuntimeWatch(const JZNodeRuntimeWatchResult &info)
{    
    QString file = info.runtimInfo.stacks.back().scriptItemPath;
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

void MainWindow::setRuntimeValue(QString file,int node_id,int pin_id,const JZNodeDebugParamValue &value)
{
    auto editor = nodeEditor(file);
    if(!editor)
        return;

    editor->setRuntimeValue(node_id,pin_id,value);
}

void MainWindow::updateRuntime(int stack_index,bool isNew)
{        
    JZNodeRuntimeInfo::Stack *stack = nullptr;
    //更新 nodeview 节点
    for (int i = stack_index; i >= 0; i--)
    {
        auto &top = m_runtime.stacks[i];
        if (!top.scriptItemPath.isEmpty() && top.function != "__idle__")
        {            
            if (i == stack_index)
            {                
                stack = &top;
            }

            setRuntimeNode(top.scriptItemPath, top.nodeId);
            break;
        }
    }
    if(isNew)
        m_stack->setRuntime(m_runtime);    
        
    auto editor_list = nodeEditorList();
    for (auto editor : editor_list)
    {
        editor->clearRuntimeValue();
    }

    if (stack)
    {
        auto func = m_program.function(stack->function);
        auto func_debug = m_program.script(stack->scriptItemPath)->functionDebug(stack->function);

        Q_ASSERT(func_debug && func_debug->nodeInfo.contains(stack->nodeId));

        JZNodeGetDebugParam param_info_watch;
        param_info_watch.stack = stack_index;

        //watch auto        
        if (func->isMemberFunction())
        {
            param_info_watch.coors << irThis();
        }
        const auto &node_info = func_debug->nodeInfo[stack->nodeId];        
        for (int i = 0; i < node_info.params.size(); i++)
        {
            int param_id = JZNodeCompiler::paramId(node_info.id, node_info.params[i].id);
            param_info_watch.coors << irId(param_id);
        }

        //manual            
        QStringList watch_list = m_watch->watchList();
        for (int i = 0; i < watch_list.size(); i++)
        {
            param_info_watch.coors << irRef(watch_list[i]);
        }

        JZNodeGetDebugParamResp param_info_watch_resp;
        if (!m_debuger.getVariable(param_info_watch, param_info_watch_resp))
            return;
              
        m_watch->setNodeInfo(node_info);
        m_watch->updateParamInfo(&param_info_watch_resp);

        auto edit = nodeEditor(stack->scriptItemPath);
        if (edit)
        {
            for (int i = 0; i < param_info_watch_resp.req.coors.size(); i++)
            {
                auto &coor = param_info_watch_resp.req.coors[i];
                if (coor.isStack())
                {
                    auto gemo = JZNodeCompiler::paramGemo(coor.id());
                    edit->setRuntimeValue(gemo.nodeId, gemo.pinId, param_info_watch_resp.values[i]);
                }
            }
        }
    }
    else
    {
        m_watch->setNodeInfo(NodeInfo());
    }
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

void MainWindow::startUnitTest(QString unitTestItemPath)
{    
    Q_ASSERT(m_processMode == Process_none);        

    auto engine = m_task.runThread()->engine();    
    m_task.addUnitTestTask(unitTestItemPath);    
}

void MainWindow::stopUnitTest()
{
    m_task.removeTask(MainTask::Task_unitTest);
}

void MainWindow::startProgram()
{    
    m_log->clearLog(Log_Runtime);   
    LOGMOD_I(Log_Runtime, "start program");

    if (m_debugSettting.type == JZDebugSetting::Local)
    {
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
    }
    else
    {
        if (!m_remote.startProgram(&m_program))
        {
            QMessageBox::information(this, "", "start failed");
            return;
        }
    }
    setRunningMode(Process_running);

    QThread::msleep(100);
    if(!m_debuger.connectToServer("127.0.0.1",19888))
    {        
        QMessageBox::information(this,"","can't connect to process");
        stopProgram();
        return;
    }
    m_log->addLog(Log_Runtime, "conenct to process");

    JZNodeDebugInfo info;
    info.breakPoints = m_project.breakPoints();
    JZNodeProgramInfo program_info; 
    if(!m_debuger.init(info,program_info))
    {
        m_log->addLog(Log_Runtime, "init process failed.");
        stopProgram();
        return;
    }
    
    m_log->addLog(Log_Runtime, "startProgram finish");
}    

void MainWindow::stopProgram()
{
    if (m_processMode == Process_none)
        return;

    if (m_debugSettting.type == JZDebugSetting::Local)
    {
        m_process.setProperty("userKill", 1);
        m_process.kill();
        m_process.waitForFinished();
    }
    else
    {        
        m_debuger.stop();        
        m_remote.stopProgram();
    }
    updateActionStatus();
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

void MainWindow::onRuntimeStatus(int status)
{        
    ProcessStatus process_status;
    if (status == Status_pause)
        process_status = Process_pause;
    else  if (status == Status_error)
        process_status = Process_error;
    else if (status == Status_none)
        process_status = Process_waitFinish;
    else if (status == Status_idle || status == Status_running)
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
        QVariantMap args;
        args["id"] = s.nodeId;
        QString line = JZNodeUtils::makeLink(s.function, s.scriptItemPath, args);
        m_log->addLog(Log_Runtime, line);
        
        line = s.function;
        if (!s.scriptItemPath.isEmpty())
            line += +"(" + s.scriptItemPath + "," + QString::number(s.nodeId) + ")";
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
            && new_runtime.stacks.back().scriptItemPath == m_runtime.stacks.back().scriptItemPath
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

void MainWindow::setWatchStatus(ProcessStatus status)
{
    m_watch->setRunningMode(status);
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