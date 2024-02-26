﻿#include "mainwindow.h"
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
#include "JZNodeEditor.h"
#include "JZUiEditor.h"
#include "JZParamEditor.h"
#include "JZNewProjectDialog.h"
#include "JZNewClassDialog.h"
#include "JZDesignerEditor.h"
#include "JZNodeUtils.h"

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
MainWindow::ActionStatus::ActionStatus(QAction *act, QVector<int> flags)
{
    this->action = act;
    this->flags = flags;
}

//MainWindow
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    m_editor = nullptr;
    m_processVaild = false;    

    connect(LogManager::instance(), &LogManager::sigLog, this, &MainWindow::onLog);

    connect(&m_debuger,&JZNodeDebugClient::sigLog,this,&MainWindow::onRuntimeLog);    
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeError,this,&MainWindow::onRuntimeError);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeStatus, this, &MainWindow::onRuntimeStatus);    
    connect(&m_debuger,&JZNodeDebugClient::sigNetError, this, &MainWindow::onNetError);
    connect(&m_debuger,&JZNodeDebugClient::sigNodePropChanged, this, &MainWindow::onNodePropChanged);

    connect(&m_process,(void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,this,&MainWindow::onRuntimeFinish);
    connect(&m_project,&JZProject::sigFileChanged, this, &MainWindow::onProjectChanged);

    loadSetting();    
    initUi();     
    updateActionStatus();    

    m_useTestProcess = 0;
    initLocalProcessTest();    
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
    auto actNewProject = menu_file->addAction("新建工程");
    auto actOpenProject = menu_file->addAction("打开工程");
    auto actCloseProject = menu_file->addAction("关闭工程");
    connect(actNewProject,&QAction::triggered,this,&MainWindow::onActionNewProject);
    connect(actOpenProject,&QAction::triggered,this,&MainWindow::onActionOpenProject);
    connect(actCloseProject,&QAction::triggered,this,&MainWindow::onActionCloseProject);
    menu_file->addSeparator();

    QMenu *menu_NewFile = menu_file->addMenu("添加");    
    auto actNewEventFile = menu_NewFile->addAction("事件");
    auto actNewClass = menu_NewFile->addAction("类");
    auto actNewFunction = menu_NewFile->addAction("函数");
    connect(actNewEventFile,&QAction::triggered,this,&MainWindow::onActionNewEvent);    
    connect(actNewClass, &QAction::triggered, this, &MainWindow::onActionNewClass);    
    connect(actNewFunction, &QAction::triggered, this, &MainWindow::onActionNewFunction);

    auto actCloseFile = menu_file->addAction("关闭文件");
    auto actSaveFile = menu_file->addAction(menuIcon("iconSave.png"), "保存文件");
    auto actSaveAllFile = menu_file->addAction(menuIcon("iconSaveAll.png"), "全部保存");
    auto actCloseAllFile = menu_file->addAction("全部关闭");

    connect(actSaveFile,&QAction::triggered,this,&MainWindow::onActionSaveFile);
    connect(actCloseFile,&QAction::triggered,this,&MainWindow::onActionCloseFile);
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

    QMenu *menu_build = menubar->addMenu("构建");
    auto actBuild = menu_build->addAction("编译");
    connect(actBuild,&QAction::triggered,this,&MainWindow::onActionBuild);
    m_actionStatus << ActionStatus(actBuild, { as::ProjectVaild, as::ProcessIsEmpty });

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
        << actStepIn << actStepOut << actBreakPoint;

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
    connect(m_log, &LogWidget::sigNodeClicked, this, &MainWindow::onNodeClicked);

    m_stack = m_log->stack();    
    connect(m_stack, &JZNodeStack::sigStackChanged, this, &MainWindow::onStackChanged);

    m_watchAuto = m_log->watchAuto();   
    m_watchManual = m_log->watchManual();    
    m_watchAuto->setReadOnly(true);    
    m_debugWidgets << m_watchAuto << m_watchManual;
    
    connect(m_watchManual, &JZNodeWatch::sigParamNameChanged, this, &MainWindow::onWatchNameChanged);
    connect(m_watchManual, &JZNodeWatch::sigParamValueChanged, this, &MainWindow::onWatchValueChanged);

    connect(m_watchAuto, &JZNodeWatch::sigParamValueChanged, this, &MainWindow::onWatchValueChanged);

    m_breakPoint = m_log->breakpoint();    

    m_projectTree = new JZProjectTree();    
    connect(m_projectTree,&JZProjectTree::sigFileOpened,this,&MainWindow::onFileOpened);
    connect(m_projectTree,&JZProjectTree::sigFileRemoved,this,&MainWindow::onFileRemoved);

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

    m_editorStack->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editorStack, &QWidget::customContextMenuRequested, this, &MainWindow::onTabContextMenu);

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
    if (m_processVaild)
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

    JZDesigner::instance()->closeEditor();
    QMainWindow::closeEvent(event);
}

void MainWindow::updateActionStatus()
{
    int status = m_runtime.status;

    bool isProject = m_project.isVaild();
    bool isEditor = (m_editor != nullptr);
    bool isEditorModify = (m_editor && m_editor->isModified());    
    bool isEditorScript = (m_editor && m_editor->type() == Editor_script);
    bool hasModifyFile = false;
    bool isProcess = m_processVaild;
    bool canPause = isProcess && (status  == Status_running || status == Status_none);
    bool canResume = isProcess && (status == Status_pause || status == Status_idlePause);
    for (auto v : m_editors)
    {
        if (v->isModified())
        {
            hasModifyFile = true;
            break;
        }
    }

    QVector<bool> cond;
    cond << isProject << isEditor << isEditorModify << isEditorScript << hasModifyFile
        << !isProcess << isProcess << canPause << canResume;
    
    Q_ASSERT(cond.size() == ActionStatus::Count);
    for (int i = 0; i < m_actionStatus.size(); i++)
    {
        auto act = m_actionStatus[i].action;
        auto &flags = m_actionStatus[i].flags;
        bool enabled = true;
        for (int j = 0; j < ActionStatus::Count; j++)
        {
            if (flags.contains(j) && !cond[j])
            {
                enabled = false;
                break;
            }
        }        
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
    if (!closeAll())
        return;

    JZNewProjectDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    
    QString name = dialog.name();
    QString project_dir = dialog.dir() + "/" + name;
    if (!QDir().exists(project_dir))
        QDir().mkpath(project_dir);
    
    if (dialog.projectType() == 0)
        m_project.initUi();
    else
        m_project.initConsole();
/*
    if (m_project.newProject(project_dir + "/" + name + ".jzproject"))
    {
        m_projectTree->setProject(&m_project);
        m_setting.addRecentProject(m_project.filePath());
    }
*/
    updateActionStatus();
}

void MainWindow::onActionOpenProject()
{    
    if (!closeProject())
        return;

    QString filepath = QFileDialog::getOpenFileName(this,"","","*.jzproject");
    if(filepath.isEmpty())
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
    closeAll();
    updateActionStatus();
}

void MainWindow::onActionCloseAllFileExcept()
{
    closeAll(true);
    updateActionStatus();
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

void MainWindow::onActionBuild()
{
    build();
}

void MainWindow::initLocalProcessTest()
{    
    if (!m_useTestProcess)
        return;
    
    connect(&m_testProcess, &QThread::finished, this, &MainWindow::onTestProcessFinish);

    m_testProcess.init(&m_project);
    m_projectTree->setProject(&m_project);    
    updateActionStatus();
}

void MainWindow::onActionRun()
{    
    start(false);
    updateActionStatus();    
}

void MainWindow::onActionDetach()
{
    m_debuger.detach();
    updateActionStatus();
    m_processVaild = false;
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
    if (!m_processVaild)
        return;

    if (!m_useTestProcess)
        m_process.kill();
    else
        m_testProcess.stop();
    updateActionStatus();
}

void MainWindow::onActionBreakPoint()
{
    if(m_editor && m_editor->type() == Editor_script)
    {
        JZNodeEditor *node_editor = (JZNodeEditor*)m_editor;
        auto ret = node_editor->breakPointTrigger();
        if(m_debuger.isConnect())
        {
            if(ret.type == BreakPointTriggerResult::add)
                m_debuger.addBreakPoint(ret.filename,ret.nodeId);
            else if(ret.type == BreakPointTriggerResult::remove)
                m_debuger.removeBreakPoint(ret.filename,ret.nodeId);
        }
        m_breakPoint->updateBreakPoint(&m_project);
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

void MainWindow::onActionHelp()
{
    QString text = "欢迎试用\nQQ群:598601341";
    QMessageBox::information(this, "", text);
}

void MainWindow::onActionCheckUpdate()
{
    QMessageBox::information(this, "", "这个功能还没做");
}

void MainWindow::onActionAbout()
{
    QString text = "欢迎试用\nQQ群:598601341";
    QMessageBox::information(this, "关于" + windowTitle(), text);
}

void MainWindow::onModifyChanged(bool flag)
{        
    auto editor = qobject_cast<JZEditor*>(sender());
    int index = m_editorStack->indexOf(editor);
    QString title = editor->item()->itemPath();
    if (flag)
        title += "*";
    m_editorStack->setTabText(index, title);
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
    if (!closeAll())
        return false;

    m_project.close();
    m_projectTree->clear();
    m_breakPoint->clear();
    updateActionStatus();
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
    m_breakPoint->updateBreakPoint(&m_project);     
    updateActionStatus();
    return true;
}

JZEditor *MainWindow::createEditor(int type)
{
    JZEditor *editor = nullptr;
    if(type == ProjectItem_scriptFlow ||type == ProjectItem_scriptParamBinding || type == ProjectItem_scriptFunction)
        editor = new JZNodeEditor();
    else if(type == ProjectItem_param)
        editor = new JZParamEditor();
    else if(type == ProjectItem_ui)
        editor = new JZUiEditor();

    if(editor)
        editor->setProject(&m_project);
    return editor;
}

void MainWindow::onFileOpened(QString filepath)
{
    openEditor(filepath);    
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
    //auto file = m_project.getFunction(functionName);
    //openEditor(file->itemPath());
}

JZEditor *MainWindow::editor(QString filepath)
{
    auto it = m_editors.find(filepath);
    if (it == m_editors.end())
        return nullptr;

    return it.value();
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
    if (!m_editors.contains(file)) {
        JZEditor *editor = createEditor(item->itemType());
        if (!editor)
            return false;

        connect(editor, &JZEditor::redoAvailable, this, &MainWindow::onRedoAvailable);
        connect(editor, &JZEditor::undoAvailable, this, &MainWindow::onUndoAvailable);
        connect(editor, &JZEditor::modifyChanged, this, &MainWindow::onModifyChanged);        
        editor->setItem(item);
        editor->open(item);
        if (editor->type() == Editor_script)
        {
            auto node_edit = (JZNodeEditor*)editor;
            connect(node_edit, &JZNodeEditor::sigFunctionOpen, this, &MainWindow::onFunctionOpen);

            node_edit->setRunning(m_processVaild);
        }
        m_editors[file] = editor;
        m_editorStack->addTab(editor, filepath);        
    }
    switchEditor(m_editors[file]);
    

    return true;
}

void MainWindow::closeEditor(JZEditor *editor)
{
    if (editor->isModified())
    {
        int ret = QMessageBox::question(this, "", "是否保存", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Yes)
        {
            editor->save();            
        }
        else if (ret == QMessageBox::Cancel)
            return;
    }    
    editor->close();
    editor->setItem(nullptr);

    int index = m_editorStack->indexOf(editor);
    m_editorStack->removeTab(index);    

    QString path = m_editors.key(editor);
    if(!path.isEmpty())
        m_editors.remove(path);
    if(m_editor == editor)
    {
        if(m_editors.size() > 0)
            switchEditor(m_editors.first());
        else
            switchEditor(nullptr);
    }
    delete editor;
}

void MainWindow::onFileClosed(QString filepath)
{
    if(!m_editors.contains(filepath))
        return;

    auto editor = m_editors[filepath];
    closeEditor(editor);
}

void MainWindow::onFileRemoved(QString filepath)
{
    onFileClosed(filepath);
    m_project.removeItem(filepath);
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
}

void MainWindow::onNodeClicked(QString file, int nodeId)
{
    if(openEditor(file))
    {
        JZNodeEditor *editor = qobject_cast<JZNodeEditor*>(m_editor);
        editor->selectNode(nodeId);
    }
}

void MainWindow::onProjectChanged()
{
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (it.value()->type() == Editor_script)
        {
            auto node_edit = (JZNodeEditor*)it.value();
            node_edit->updateNode();
        }

        it++;
    }
}

void MainWindow::onStackChanged(int stack_index)
{
    updateRuntime(stack_index, false);
}

void MainWindow::onWatchValueChanged(JZNodeParamCoor coor, QVariant value)
{    
    JZNodeSetDebugParamInfo param_info;
    param_info.stack = m_stack->stackIndex();
    param_info.coor = coor;
    param_info.value = value;
    
    JZNodeWatch *watch = qobject_cast<JZNodeWatch*>(sender());
    JZNodeDebugParamInfo result = m_debuger.setVariable(param_info);
    watch->updateParamInfo(&result);

    if (coor.type == JZNodeParamCoor::Id)
    {
        auto stack_info = m_runtime.stacks[param_info.stack];

        JZNodeValueChanged info;
        info.file = stack_info.file;
        info.id = coor.id;
        info.value = result.values[0].value;
        onNodePropChanged(info);
    }
}

void MainWindow::onWatchNameChanged(JZNodeParamCoor coor)
{    
    JZNodeDebugParamInfo param_info;
    param_info.stack = m_stack->stackIndex();
    param_info.coors << coor;
    
    param_info = m_debuger.getVariable(param_info);
    m_watchManual->updateParamInfo(&param_info);        
}

void MainWindow::onNodePropChanged(const JZNodeValueChanged &info)
{
    auto edit = editor(info.file);
    if (!edit)
        return;

    auto gemo = JZNodeCompiler::paramGemo(info.id);
    JZNodeEditor *node_editor = qobject_cast<JZNodeEditor*>(edit);
    node_editor->setNodeValue(gemo.nodeId, gemo.propId,info.value);
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
    
    //watch auto
    JZNodeDebugParamInfo param_info;      
    param_info.stack = stack_index;

    //this
    auto func = m_program.function(stack.function);    
    if (func->isCFunction)
    {
        for(int i = stack_index - 1; i >= 0; i--)
        {
            auto &top = m_runtime.stacks[i];
            if (!top.file.isEmpty())
            {
                setRuntimeNode(top.file, top.nodeId);
                break;
            }
        }

        m_watchAuto->setParamInfo(&param_info);        
        return;
    }

    setRuntimeNode(stack.file, stack.nodeId);
    if (func->isMemberFunction())
    {        
        JZNodeParamCoor coor;
        coor.type = JZNodeParamCoor::Name;
        coor.name = "this";
        param_info.coors << coor;        
    }

    //local
    auto &runtime = m_program.scripts[stack.file].runtimeInfo[stack.function];
    for (int i = 0; i < runtime.localVariables.size(); i++)
    {
        auto &local = runtime.localVariables[i];

        JZNodeParamCoor coor;
        coor.type = JZNodeParamCoor::Name;
        coor.name = local.name;
        param_info.coors << coor;
    }
    
    auto &node_info = m_program.scripts[stack.file].nodeInfo[stack.nodeId];
    int node_prop_index = param_info.coors.size();
    for (int i = 0; i < node_info.paramIn.size(); i++)
    {
        JZNodeParamCoor coor;
        coor.type = JZNodeParamCoor::Id;
        coor.name = node_info.paramIn[i].define.name;
        coor.id = node_info.paramIn[i].id;
        param_info.coors << coor;
    }    
    for (int i = 0; i < node_info.paramOut.size(); i++)
    {
        JZNodeParamCoor coor;
        coor.type = JZNodeParamCoor::Id;
        coor.name = node_info.paramOut[i].define.name;
        coor.id = node_info.paramOut[i].id;
        param_info.coors << coor;
    }

    param_info = m_debuger.getVariable(param_info);
    m_watchAuto->setParamInfo(&param_info);

    for (int i = node_prop_index; i < param_info.coors.size(); i++)
    {
        JZNodeValueChanged info;
        info.file = stack.file;
        info.id = param_info.coors[i].id;
        info.value = param_info.values[i].value;
        onNodePropChanged(info);
    }

    //watch manual    
    JZNodeDebugParamInfo param_info_watch;
    param_info_watch.stack = stack_index;

    QStringList watch_list = m_watchManual->watchList();
    for (int i = 0; i < watch_list.size(); i++)
    {
        JZNodeParamCoor coor;
        coor.type = JZNodeParamCoor::Name;        
        coor.name = watch_list[i];
        param_info_watch.coors << coor;
    }
    param_info_watch = m_debuger.getVariable(param_info_watch);
    m_watchManual->setParamInfo(&param_info_watch);

    //watch reg
    JZNodeDebugParamInfo param_info_reg;
    param_info_reg.stack = stack_index;

    QList<int> reg_list = {Reg_Cmp};
    for (int i = 0; i < watch_list.size(); i++)
    {
        JZNodeParamCoor coor;
        coor.type = JZNodeParamCoor::Id;
        coor.id = reg_list[i];
        param_info_reg.coors << coor;
    }
    param_info_reg = m_debuger.getVariable(param_info_reg);    
}

bool MainWindow::build()
{
    QElapsedTimer timer;
    timer.start();
    m_log->addLog(Log_Compiler, "开始编译");

    JZNodeBuilder builder;
    JZNodeProgram program;
    if(!builder.build(&m_project,&program))
    {        
        m_log->addLog(Log_Compiler, "编译失败\n");
        return false;
    }
    QString build_path = m_project.path() + "/build";
    QString build_exe = build_path + "/" + m_project.name() + ".program";
    QDir dir;
    if (!dir.exists(build_path))
        dir.mkdir(build_path);
    if (!program.save(build_exe))
    {
        m_log->addLog(Log_Compiler, "generate program failed");
        return false;
    }
    saveToFile(build_path + "/" + m_project.name() + ".jsm", program.dump());

    m_log->addLog(Log_Compiler, "编译完成 -> " + build_exe + ", 用时" + QString::number(timer.elapsed()) + "ms");
    return true;
}

void MainWindow::saveToFile(QString filepath,QString text)
{
    QFile file(filepath);
    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream s(&file);
        s << text;
        file.close();
    }
}

void MainWindow::start(bool startPause)
{    
    if(!build())
        return;       

    if (!m_useTestProcess)
    {
        QString app = qApp->applicationFilePath();
        QString build_exe = m_project.path() + "/build/" + m_project.name() + ".program";
        QStringList params;
        params << "--run" << build_exe << "--debug";
        if (startPause)
            params << "--start-pause";

        m_log->addLog(Log_Runtime, "start program");
        m_process.start(app, params);
        if (!m_process.waitForStarted())
        {
            QMessageBox::information(this, "", "start failed");
            return;
        }
    }
    else
    {
        m_log->addLog(Log_Runtime, "start program");
        m_testProcess.start();
    }
    m_processVaild = true;

    QThread::msleep(500);
    if(!m_debuger.connectToServer("127.0.0.1",19888))
    {
        QMessageBox::information(this,"","can't connect to process");
        return;
    }
    m_log->addLog(Log_Runtime, "conenct to server");

    JZNodeDebugInfo info;
    info.breakPoints = m_project.breakPoints();
    m_program = m_debuger.init(info);

    setRunning(true);    
}

void MainWindow::onTabContextMenu(QPoint pos)
{
    QMenu menu(this);
    QAction *actSave = menu.addAction("保存");
    QAction *actClose = menu.addAction("关闭");
    QAction *actAll = menu.addAction("关闭所有文档");
    QAction *actAllExcept = menu.addAction("除此之外全部关闭");

    QAction *ret = menu.exec(m_editorStack->mapToGlobal(pos));
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
        onActionCloseAllFileExcept();
    }
}

void MainWindow::onLog(LogObjectPtr log)
{
    m_log->addLog(log->module, log->message);
}

void MainWindow::onRuntimeStatus(int status)
{        
    bool is_pause = statusIsPause(status);
    setWatchStatus(status);

    if (is_pause)
    {
        auto new_runtime = m_debuger.runtimeInfo();
        bool isNew = true;
        if (new_runtime.stacks.size() > 0 && new_runtime.stacks.size() == m_runtime.stacks.size()
            && new_runtime.stacks.back().file == m_runtime.stacks.back().file
            && new_runtime.stacks.back().function == m_runtime.stacks.back().function)
        {
            isNew = false;
        }

        m_runtime = new_runtime;
        m_log->stack()->setRuntime(m_runtime);
        updateRuntime(m_runtime.stacks.size() - 1, isNew);
    }
    else
    {
        m_runtime.status = status;
        m_log->stack()->setRuntime(m_runtime);
    }

    if (!is_pause)
        clearRuntimeNode();

    updateActionStatus();
}

void MainWindow::onRuntimeLog(QString log)
{
    m_log->addLog(Log_Runtime, log);
}

void MainWindow::onRuntimeError(JZNodeRuntimeError error)
{
    QString error_msg = "Runtime Error: " + error.error + "\n\n";
    m_log->addLog(Log_Runtime, error_msg);
    
    int stack_size = error.info.stacks.size();
    for (int i = 0; i < stack_size; i++)
    {
        auto s = error.info.stacks[stack_size - i - 1];
        QString line = makeLink(s.file, s.function, s.nodeId);
        m_log->addLog(Log_Runtime, line);
        
        line = s.function;
        if (!s.file.isEmpty())
            line += +"(" + s.file + "," + QString::number(s.nodeId) + ")";        
        error_msg += line + "\n";
    }    

    activateWindow();    
    QMessageBox::information(this, "", error_msg);    
}

void MainWindow::onNetError()
{
    return;
    m_log->addLog(Log_Runtime, "调试连接中断");
    onActionStop();
}

void MainWindow::onTestProcessFinish()
{
    m_log->addLog(Log_Runtime, "local server test finish.");
    setRunning(false);
    m_processVaild = false;
    updateActionStatus();
}

void MainWindow::onRuntimeFinish(int code,QProcess::ExitStatus status)
{
    setRunning(false);

    if(status == QProcess::CrashExit)
        m_log->addLog(Log_Runtime, "process crash ");
    else
        m_log->addLog(Log_Runtime, "process finish, exit code " + QString::number(code));
    m_processVaild = false;
    updateActionStatus();
}

void MainWindow::saveAll()
{
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (editor->isModified())
            editor->save();

        it++;
    }
}

bool MainWindow::closeAll(bool except_current)
{
    QList<JZEditor*> close_list;
    QStringList close_file_list;

    bool saveToAll = false, noToAll = false;
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (except_current && m_editor == editor)
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
                if (ret == QMessageBox::Yes)
                {
                    editor->save();                    
                }
                else if (ret == QMessageBox::Cancel)
                    return false;
                else if (ret == QMessageBox::YesToAll)
                    saveToAll = true;
                else if (ret == QMessageBox::NoToAll)
                    noToAll = true;
            }
            else if (saveToAll)
            {
                editor->save();                
            }
        }
        editor->close();
        close_list << editor;
        close_file_list << it.key();

        int index = m_editorStack->indexOf(editor);
        m_editorStack->removeTab(index);
        it++;
    }
    for (auto editor : close_list)
        delete editor;
    for (auto editor_path : close_file_list)
        m_editors.remove(editor_path);
    if (!except_current)
    {
        m_editor = nullptr;
        switchEditor(nullptr);
    }
    return true;
}

void MainWindow::setRunning(bool flag)
{
    setWatchRunning(flag);    

    m_stack->setRunning(flag);

    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        if (it.value()->type() == Editor_script)
        {
            auto node_edit = (JZNodeEditor*)it.value();
            node_edit->setRunning(flag);
        }        
        it++;
    }

    if (!flag)
    {
        m_program = JZNodeProgramInfo();
        m_runtime = JZNodeRuntimeInfo();
    }
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

void MainWindow::setWatchStatus(int status)
{
    for (auto w : m_debugWidgets)
        w->setRuntimeStatus(status);
}

void MainWindow::setWatchRunning(bool flag)
{
    for (auto w : m_debugWidgets)
        w->setRunning(flag);
}