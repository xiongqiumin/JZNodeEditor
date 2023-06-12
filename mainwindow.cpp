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
#include "JZNodeEditor.h"
#include "JZUiEditor.h"
#include "JZParamEditor.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    m_editor = nullptr;
    m_actRun = m_actDetach = m_actPause = m_actResume = m_actStop = nullptr;
    m_actStepOver = m_actStepIn = m_actStepOut = m_actBreakPoint = nullptr;

    connect(&m_debuger,&JZNodeDebugClient::sigLog,this,&MainWindow::onDebugLog);
    connect(&m_debuger,&JZNodeDebugClient::sigRuntimeError,this,&MainWindow::onRuntimeError);

    connect(&m_process,(void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,this,&MainWindow::onDebugFinish);

    initMenu();
    initUi(); 
    updateMenuAction();

    resize(800, 600);    
    //onActionOpenProject();
}

MainWindow::~MainWindow()
{
    for(auto edit : m_editors)
        edit->disconnect();
}

void MainWindow::initMenu()
{
    QMenuBar *menubar = new QMenuBar();
    this->setMenuBar(menubar);

    QMenu *menu_file = menubar->addMenu("文件");
    auto actNew = menu_file->addAction("新建");
    auto actOpen = menu_file->addAction("打开");
    auto actClose = menu_file->addAction("关闭");
    auto actSave = menu_file->addAction("保存");
    auto actSaveAs = menu_file->addAction("另存为");
    connect(actNew,&QAction::triggered,this,&MainWindow::onActionNewProject);
    connect(actOpen,&QAction::triggered,this,&MainWindow::onActionOpenProject);
    connect(actClose,&QAction::triggered,this,&MainWindow::onActionCloseProject);
    connect(actSave,&QAction::triggered,this,&MainWindow::onActionSaveProject);
    connect(actSaveAs,&QAction::triggered,this,&MainWindow::onActionSaveAsProject);

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
    m_actRun = menu_debug->addAction("开始调试");
    m_actDetach = menu_debug->addAction("脱离调试器");
    m_actPause = menu_debug->addAction("中断");
    m_actResume = menu_debug->addAction("继续");
    m_actStop = menu_debug->addAction("停止调试");

    menu_debug->addSeparator();
    m_actStepOver = menu_debug->addAction("单步");
    m_actStepIn = menu_debug->addAction("单步进入");
    m_actStepOut = menu_debug->addAction("单步跳出");
    m_actBreakPoint = menu_debug->addAction("断点");
    m_actRun->setShortcut(QKeySequence("F5"));
    m_actStepOver->setShortcut(QKeySequence("F0"));
    m_actStepIn->setShortcut(QKeySequence("F11"));
    m_actStepOut->setShortcut(QKeySequence("Shift+F11"));
    m_actBreakPoint->setShortcut(QKeySequence("F9"));       

    connect(m_actRun,&QAction::triggered,this,&MainWindow::onActionRun);
    connect(m_actDetach,&QAction::triggered,this,&MainWindow::onActionDetach);
    connect(m_actPause,&QAction::triggered,this,&MainWindow::onActionPause);
    connect(m_actResume,&QAction::triggered,this,&MainWindow::onActionResume);
    connect(m_actStop,&QAction::triggered,this,&MainWindow::onActionStop);

    connect(m_actStepOver,&QAction::triggered,this,&MainWindow::onActionStepOver);
    connect(m_actStepIn,&QAction::triggered,this,&MainWindow::onActionStepIn);
    connect(m_actStepOut,&QAction::triggered,this,&MainWindow::onActionStepOut);
    connect(m_actBreakPoint,&QAction::triggered,this,&MainWindow::onActionBreakPoint);

    QMenu *menu_help = menubar->addMenu("帮助");
    menu_help->addAction("帮助");
    menu_help->addSeparator();
    menu_help->addAction("检查更新");       

    m_menuList << menu_file << menu_edit << menu_view << menu_build << menu_debug << menu_help;
}

void MainWindow::initUi()
{    
    m_log = new QTextEdit();
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
    m_editorStack = new QStackedWidget();
    m_editorStack->addWidget(new QLabel("empty"));
    m_editorStack->addWidget(node_editor);    

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

void MainWindow::updateMenuAction()
{
    QMenu *menu_edit = m_menuList[1];
    auto edit_act_list = menu_edit->actions();
    edit_act_list[0]->setEnabled(false);
    edit_act_list[1]->setEnabled(false);
    edit_act_list[2]->setEnabled(false);
    edit_act_list[3]->setEnabled(false);
    edit_act_list[4]->setEnabled(false);

    QMenu *menu_debug = m_menuList[4];
    auto debug_act_list = menu_debug->actions();
}

void MainWindow::onActionNewProject()
{
    closeAll();
    m_project.initUi();
    m_projectTree->setProject(&m_project);
}

void MainWindow::onActionOpenProject()
{    
/*
    QString filepath = QFileDialog::getOpenFileName(this,"","","*.jzproject");
    if(filepath.isEmpty())
        return;
*/
    closeAll();
    QString filepath = "test.prj";
    if(!QFile::exists(filepath))
        m_project.saveAs(filepath);
    if(m_project.open(filepath))
        m_projectTree->setProject(&m_project);
}

void MainWindow::onActionCloseProject()
{
    m_project.save();
}

void MainWindow::onActionSaveProject()
{

}

void MainWindow::onActionSaveAsProject()
{
    QString filepath = QFileDialog::getSaveFileName(this);
    if(filepath.isEmpty())
        return;

    m_project.saveAs(filepath);
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
}

void MainWindow::onActionDetach()
{
    m_debuger.detach();
}

void MainWindow::onActionPause()
{
    m_debuger.pause();    
}

void MainWindow::onActionResume()
{
    m_debuger.resume();
}

void MainWindow::onActionStop()
{    
    m_process.kill();
}

void MainWindow::onActionBreakPoint()
{

}

void MainWindow::onActionStepOver()
{
    m_debuger.stepOver();
}

void MainWindow::onActionStepIn()
{
    m_debuger.stepIn();
}

void MainWindow::onActionStepOut()
{
    m_debuger.stepOut();
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
    if(type == ProjectItem_scriptFlow ||type == ProjectItem_scriptParamBinding || type == ProjectItem_scriptFunction)
        return new JZNodeEditor();
    else if(type == ProjectItem_param)
        return new JZParamEditor();
    else if(type == ProjectItem_ui)
        return new JZUiEditor();

    return nullptr;
}

void MainWindow::onFileOpened(QString filepath)
{
    JZProjectItem *item = m_project.getItem(filepath);
    QString file = item->itemPath();
    if(!m_editors.contains(file)){
        JZEditor *editor = createEditor(item->itemType());
        if(!editor)
            return;

        connect(editor,&JZEditor::redoAvailable,this,&MainWindow::onRedoAvailable);
        connect(editor,&JZEditor::undoAvailable,this,&MainWindow::onUndoAvailable);
        m_editorStack->addWidget(editor);
        m_editors[file] = editor;        
        m_editors[file]->open(item);
    }    
    switchEditor(m_editors[file]);
}

void MainWindow::switchEditor(JZEditor *editor)
{
    m_editor = editor;
    if(editor != nullptr)
    {
        m_editorStack->setCurrentWidget(m_editor);
        m_editor->updateMenuBar(this->menuBar());
    }
    else
        m_editorStack->setCurrentIndex(0);
}

void MainWindow::onFileClosed(QString filepath)
{
    if(!m_editors.contains(filepath))
        return;

    auto editor = m_editors[filepath];
    editor->close();
    m_editorStack->removeWidget(editor);
    delete editor;
    m_editors.remove(filepath);
    if(m_editor == editor)
    {
        if(m_editors.size() == 0)
            switchEditor(m_editors.first());
        else
            switchEditor(nullptr);
    }
}

bool MainWindow::build()
{
    if(!m_builder.build(&m_project,&m_program))
    {
        m_log->append(m_builder.error());
        m_log->append("编译失败");
        return false;
    }

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
    QString app_dir = qApp->applicationDirPath();

    if(!m_builder.build(&m_project,&m_program))
    {
        m_log->append("build failed");
        return;
    }

    QString build_path = app_dir + "/build";
    QString build_exe = build_path + "/" + m_project.name() + ".program";    
    QDir dir;
    if(!dir.exists(build_path))
        dir.mkdir(build_path);
    if(!m_program.save(build_exe))
    {
        m_log->append("generate program failed");
        return;
    }
    saveToFile(build_path + "/" + m_project.name() + ".jsm",m_program.dump());

    QStringList params;
    params << "--run" << build_exe << "--debug";
    if(startPause)
        params << "--start-pause";

    m_log->append("start program");
    m_process.start(app,params);
    if(!m_process.waitForStarted())
    {
        QMessageBox::information(this,"","start failed");
        return;
    }

    QThread::msleep(500);
    if(!m_debuger.connectToServer("127.0.0.1",19888))
    {
        QMessageBox::information(this,"","can't connect to process");
        return;
    }
    m_log->append("conenct to server");
}

void MainWindow::onDebugLog(QString log)
{
    m_log->append(log);
}

void MainWindow::onRuntimeError(JZNodeRuntimeError error)
{
    m_log->append("Runtime error:");
    m_log->append(error.info);
}

void MainWindow::onDebugFinish(int code,QProcess::ExitStatus status)
{
    if(status == QProcess::CrashExit)
        m_log->append("process crash ");
    else
        m_log->append("process finish, exit code " + QString::number(code));
}

void MainWindow::closeAll()
{
    auto it = m_editors.begin();
    while(it != m_editors.end())
    {
        it.value()->close();
        m_editorStack->removeWidget(it.value());
        delete it.value();
        it++;
    }
    m_editors.clear();
    switchEditor(nullptr);
}
