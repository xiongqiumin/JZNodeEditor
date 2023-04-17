#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    initUi(); 
    resize(800, 600);

    onActionOpenProject();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUi()
{    
    m_log = new QTextEdit();
    m_projectTree = new JZProjectTree();    

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
    m_editorStack = new QStackedWidget();
    m_editorStack->addWidget(new QLabel("empty"));

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

void MainWindow::onActionOpenProject()
{
    m_projectTree->setProject(&m_project);
}

void MainWindow::onActionSaveProject()
{

}

void MainWindow::onActionOpenTriggered()
{
    QString filepath = QFileDialog::getOpenFileName(this,"","","*.jzproject");
    if(filepath.isEmpty())
        return;

    m_projectTree->update();
}

void MainWindow::onActionSaveTriggered()    
{

}

void MainWindow::onActionSaveAsTriggered()
{
    QString filepath = QFileDialog::getSaveFileName(this);
    if(filepath.isEmpty())
        return;

}

void MainWindow::onActionSaveAllTriggered()
{

}

void MainWindow::onFileOpened(QString filepath)
{
    JZProjectItem *item = m_project.getItem(filepath);
    //m_editor->setFile(item);
}

void MainWindow::onSetValue(int id, QVariant value)
{
/*
    if(m_vm.isRunning())
    {
        m_vm.setVariable(id,value);

        JZEvent *event = new JZEvent();
        event->setEventType(Event_paramChanged);
        event->params << QString::number(id);
        qApp->sendEvent(&m_vm,event);
    }
*/
}

void MainWindow::onDispValue(int id, QVariant value)
{
/*
    auto gemo = m_program.paramGemo(id);
    if(!gemo.isNull())
    {
        auto item = getNodeItem(gemo.nodeId);
        item->widget()->setVariable(id,value);
    }
*/
}
