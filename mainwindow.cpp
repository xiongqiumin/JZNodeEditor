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
#include "JZNodeEditor.h"
#include "JZNodeEditor.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    m_editor = nullptr;

    initMenu();
    initUi(); 
    resize(800, 600);    
}

MainWindow::~MainWindow()
{
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
    actUndo->setShortcut(QKeySequence("Ctrl+Z"));
    actRedo->setShortcut(QKeySequence("Ctrl+Y"));
    actDel->setShortcut(QKeySequence("Ctrl+D"));
    actCut->setShortcut(QKeySequence("Ctrl+X"));
    actCopy->setShortcut(QKeySequence("Ctrl+C"));
    actPaste->setShortcut(QKeySequence("Ctrl+V"));

    connect(actUndo,&QAction::triggered,this,&MainWindow::onActionUndo);
    connect(actRedo,&QAction::triggered,this,&MainWindow::onActionRedo);
    connect(actDel,&QAction::triggered,this,&MainWindow::onActionDel);
    connect(actCut,&QAction::triggered,this,&MainWindow::onActionCut);
    connect(actCopy,&QAction::triggered,this,&MainWindow::onActionCopy);
    connect(actPaste,&QAction::triggered,this,&MainWindow::onActionPaste);

    QMenu *menu_view = menubar->addMenu("视图");
    menu_view->addAction("显示窗口");
    menu_view->addAction("恢复默认");

    QMenu *menu_build = menubar->addMenu("构建");
    auto actBuild = menu_build->addAction("编译");
    connect(actBuild,&QAction::triggered,this,&MainWindow::onActionBuild);

    QMenu *menu_debug = menubar->addMenu("调试");
    auto actRun = menu_debug->addAction("运行");
    auto actStep = menu_debug->addAction("单步");
    auto actStepIn = menu_debug->addAction("单步进入");
    auto actStepOver = menu_debug->addAction("单步跳出");
    actRun->setShortcut(QKeySequence("F5"));
    actRun->setShortcut(QKeySequence("F0"));
    actRun->setShortcut(QKeySequence("F11"));
    actRun->setShortcut(QKeySequence("Shift+F11"));

    connect(actRun,&QAction::triggered,this,&MainWindow::onActionRun);
    connect(actStep,&QAction::triggered,this,&MainWindow::onActionStepIn);
    connect(actStepIn,&QAction::triggered,this,&MainWindow::onActionStepOut);
    connect(actStepOver,&QAction::triggered,this,&MainWindow::onActionStepOver);

    QMenu *menu_help = menubar->addMenu("帮助");
    menu_help->addAction("帮助");
    menu_help->addSeparator();
    menu_help->addAction("检查更新");
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
    splitterMain->setStretchFactor(0,1);
    splitterMain->setStretchFactor(1,3);

    splitterLeft->setCollapsible(0,false);
    splitterLeft->setCollapsible(1,false);
    splitterLeft->setStretchFactor(0,3);
    splitterLeft->setStretchFactor(1,1);

    setCentralWidget(widget);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

void MainWindow::onActionNewProject()
{
    m_project.init();
    m_projectTree->setProject(&m_project);
}

void MainWindow::onActionOpenProject()
{
    QString filepath = QFileDialog::getOpenFileName(this,"","","*.jzproject");
    if(filepath.isEmpty())
        return;

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

void MainWindow::onActionBuild()
{
    if(!m_builder.build(&m_project,&m_program))
        m_log->append("编译失败");
}

void MainWindow::onActionRun()
{

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

JZEditor *MainWindow::createEditor(int type)
{
    if(type == ProjectItem_scriptFlow ||type == ProjectItem_scriptParam)
        return new JZNodeEditor();

    return nullptr;
}

void MainWindow::onFileOpened(QString filepath)
{
    JZProjectItem *item = m_project.getItem(filepath);
    QString file = item->path();
    if(!m_editors.contains(file)){
        JZEditor *editor = createEditor(item->itemType());
        if(!editor)
            return;

        m_editorStack->addWidget(editor);
        m_editors[file] = editor;
    }
    m_editors[file]->open(item);
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
