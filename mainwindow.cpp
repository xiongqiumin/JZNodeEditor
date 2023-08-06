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
#include "JZNodeEditor.h"
#include "JZUiEditor.h"
#include "JZParamEditor.h"
#include "JZNewProjectDialog.h"
#include "JZDesignerEditor.h"

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

    connect(&m_debuger,&JZNodeDebugClient::sigLog,this,&MainWindow::onRuntimeLog);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeInfo,this,&MainWindow::onRuntimeInfo);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeError,this,&MainWindow::onRuntimeError);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeStatus, this, &MainWindow::onRuntimeStatus);    

    connect(&m_process,(void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,this,&MainWindow::onDebugFinish);

    loadSetting();
    initMenu();
    initUi();     
    updateActionStatus();    
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

    QMenu *menu_NewFile = menu_file->addMenu("新建文件");    
    auto actNewFile = menu_NewFile->addAction("项目");
    connect(actNewFile,&QAction::triggered,this,&MainWindow::onActionNewFile);
    auto actNewClass = menu_NewFile->addAction("类");
    connect(actNewClass, &QAction::triggered, this, &MainWindow::onActionNewFile);

    auto actCloseFile = menu_file->addAction("关闭文件");
    auto actSaveFile = menu_file->addAction("保存文件");
    auto actSaveAllFile = menu_file->addAction("全部保存");
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
        << ActionStatus(actSaveAllFile, { as::ProjectVaild })
        << ActionStatus(actCloseAllFile, { as::FileOpen });

    QMenu *menu_edit = menubar->addMenu("编辑");
    auto actUndo = menu_edit->addAction("撤销");
    auto actRedo = menu_edit->addAction("重做");
    menu_edit->addSeparator();
    auto actDel = menu_edit->addAction("删除");
    auto actCut = menu_edit->addAction("剪切");
    auto actCopy = menu_edit->addAction("复制");
    auto actPaste = menu_edit->addAction("粘贴");
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

    QMenu *menu_debug = menubar->addMenu("调试");    
    auto actRun = menu_debug->addAction("开始调试");
    auto actDetach = menu_debug->addAction("脱离调试器");
    auto actPause = menu_debug->addAction("中断");
    auto actResume = menu_debug->addAction("继续");
    auto actStop = menu_debug->addAction("停止调试");

    menu_debug->addSeparator();
    auto actStepOver = menu_debug->addAction("单步");
    auto actStepIn = menu_debug->addAction("单步进入");
    auto actStepOut = menu_debug->addAction("单步跳出");
    auto actBreakPoint = menu_debug->addAction("断点");
    actRun->setShortcut(QKeySequence("F5"));
    actStepOver->setShortcut(QKeySequence("F10"));
    actStepIn->setShortcut(QKeySequence("F11"));
    actStepOut->setShortcut(QKeySequence("Shift+F11"));
    actBreakPoint->setShortcut(QKeySequence("F9"));

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
    menu_help->addAction("帮助");
    menu_help->addSeparator();
    menu_help->addAction("检查更新");       

    m_menuList << menu_file << menu_edit << menu_view << menu_build << menu_debug << menu_help;
}

void MainWindow::initUi()
{    
    m_log = new LogWidget();
    connect(m_log, &LogWidget::sigNodeClicked, this, &MainWindow::onNodeClicked);

    m_projectTree = new JZProjectTree();    
    connect(m_projectTree,&JZProjectTree::sigFileOpened,this,&MainWindow::onFileOpened);

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

    QSplitter *splitterLeft = new QSplitter(Qt::Vertical);
    splitterLeft->addWidget(m_editorStack);    
    splitterLeft->addWidget(m_log);       
    l_left->addWidget(splitterLeft);

    splitterMain->setCollapsible(0,false);
    splitterMain->setCollapsible(1,false);
    splitterMain->setStretchFactor(0,0);
    splitterMain->setStretchFactor(1,1);
    splitterMain->setSizes({150,600});

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
    if (!closeAll())
    {
        event->ignore();
        return;
    }
    JZDesigner::instance()->closeEditor();
    QMainWindow::closeEvent(event);
}

void MainWindow::updateActionStatus()
{
    int status = m_debuger.status();

    bool isProject = m_project.isVaild();
    bool isEditor = (m_editor != nullptr);
    bool isEditorModify = (m_editor && m_editor->isModified());    
    bool isEditorScript = (m_editor && m_editor->type() == Editor_script);
    bool isProcess = m_processVaild;
    bool canPause = isProcess && (status  == Status_running || status == Status_none);
    bool canResume = (status == Status_pause || status == Status_idlePause);    

    QVector<bool> cond;
    cond << isProject << isEditor << isEditorModify << isEditorScript
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
    
    m_project.initUi();
    if (m_project.saveAs(project_dir + "/" + name + ".jzproject"))
    {
        m_projectTree->setProject(&m_project);
        m_setting.addRecentProject(m_project.filePath());
    }
    updateActionStatus();
}

void MainWindow::onActionOpenProject()
{    
    if (!closeAll())
        return;

    QString filepath = QFileDialog::getOpenFileName(this,"","","*.jzproject");
    if(filepath.isEmpty())
        return;
        
    if (m_project.open(filepath))
    {        
        m_projectTree->setProject(&m_project);
        m_setting.addRecentProject(m_project.filePath());        
    }
    updateActionStatus();
}

void MainWindow::onActionCloseProject()
{
    if (!closeAll())
        return;

    m_project.close();    
    m_projectTree->clear();
    updateActionStatus();
}

void MainWindow::onActionRecentProject()
{
    QAction *act = qobject_cast<QAction*>(sender());
    QString filepath = act->text();

    closeAll();
    if (m_project.open(filepath))
    {        
        m_projectTree->setProject(&m_project);
        m_setting.addRecentProject(m_project.filePath());
        updateActionStatus();
    }
    else
    {
        m_setting.recentFile.removeAll(filepath);
        QMenu *menu = qobject_cast<QMenu *>(act->parent());
        menu->removeAction(act);  
        delete act;
    }
}

void MainWindow::onActionNewFile()
{

}

void MainWindow::onActionSaveFile()
{
    if(!m_editor)
        return;

    m_editor->save();
    m_project.save();
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
    m_process.kill();
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
    JZProjectItem *item = m_project.getItem(filepath);
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
            m_project.save();
        }
        else if (ret == QMessageBox::Cancel)
            return;
    }
    editor->close();

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
        editor->ensureNodeVisible(nodeId);
    }
}

bool MainWindow::build()
{
    m_log->addLog(Log_Compiler, "开始编译");
    if(!m_builder.build(&m_project,&m_program))
    {
        m_log->addLog(Log_Compiler,m_builder.error());
        m_log->addLog(Log_Compiler, "编译失败");
        return false;
    }
    QString build_path = m_project.path() + "/build";
    QString build_exe = build_path + "/" + m_project.name() + ".program";
    QDir dir;
    if (!dir.exists(build_path))
        dir.mkdir(build_path);
    if (!m_program.save(build_exe))
    {
        m_log->addLog(Log_Compiler, "generate program failed");
        return false;
    }
    saveToFile(build_path + "/" + m_project.name() + ".jsm", m_program.dump());

    m_log->addLog(Log_Compiler, "编译完成:" + build_exe);
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

    QString app = qApp->applicationFilePath();
    QString build_exe = m_project.path() + "/build/" + m_project.name() + ".program";
    QStringList params;
    params << "--run" << build_exe << "--debug";
    if(startPause)
        params << "--start-pause";

    m_log->addLog(Log_Runtime, "start program");
    m_process.start(app,params);
    if(!m_process.waitForStarted())
    {
        QMessageBox::information(this,"","start failed");
        return;
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
    m_debuger.init(info);
}

void MainWindow::onRuntimeStatus(int status)
{
    m_log->addLog(Log_Runtime, QString("status:") + QString::number(status));
    auto it = m_editors.begin();
    while (it != m_editors.end())
    {
        auto editor = it.value();
        if (editor->type() == Editor_script)
            ((JZNodeEditor*)editor)->setRuntimeStatus(status);

        it++;
    }
    updateActionStatus();
}

void MainWindow::onRuntimeLog(QString log)
{
    m_log->addLog(Log_Runtime, log);
}

void MainWindow::onRuntimeInfo(JZNodeRuntimeInfo info)
{    
    if (openEditor(info.file))
    {
        JZNodeEditor *editor = qobject_cast<JZNodeEditor*>(m_editor);
        editor->setRuntimeNode(info.nodeId);        
    }
    updateActionStatus();
}

void MainWindow::onRuntimeError(JZNodeRuntimeError error)
{
    m_log->addLog(Log_Runtime, "Runtime error:");
    m_log->addLog(Log_Runtime, error.info);
}

void MainWindow::onDebugFinish(int code,QProcess::ExitStatus status)
{
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
    m_project.saveAllItem();
    m_project.save();
}

bool MainWindow::closeAll()
{
    bool saveToAll = false, noToAll = false;
    auto it = m_editors.begin();
    while(it != m_editors.end())
    {
        auto editor = it.value();
        if (editor->isModified())
        {
            if (!saveToAll && !noToAll)
            {
                int ret = QMessageBox::question(this, "", "是否保存", QMessageBox::Yes | QMessageBox::No
                    | QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel);
                if (ret == QMessageBox::Yes)
                {
                    editor->save();
                    m_project.save();
                }
                else if (ret == QMessageBox::Cancel)
                    return false;
                else if (ret == QMessageBox::YesToAll)
                    saveToAll = true;
                else if (ret == QMessageBox::NoToAll)
                    noToAll = true;
            }
            else if (saveToAll)
                editor->save();
        }
        editor->close();

        int index = m_editorStack->indexOf(editor);
        m_editorStack->removeTab(index);
        if (editor == m_editor)
            switchEditor(nullptr);
        delete editor;
        it++;
    }
    m_editors.clear();    
    return true;
}
