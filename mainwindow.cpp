#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    initUi();
 
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUi()
{    
    m_log = new QTextEdit();
    m_projectTree = new JZProjectTree();    

    QVBoxLayout *center = new QVBoxLayout();     

    QWidget *w_left = new QWidget();
    QVBoxLayout *l_left = new QVBoxLayout();
    l_left->setContentsMargins(0,0,0,0);
    w_left->setLayout(l_left);

    //main
    QSplitter *splitterMain = new QSplitter(Qt::Horizontal);
    splitterMain->addWidget(m_projectTree);
    splitterMain->addWidget(w_left);

    center->addWidget(splitterMain);    

    //left
    m_editorStack = new QStackWidget();

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
}

void MainWindow::onActionOpenProject()
{

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

void MainWindow::on_actionSaveAs_triggered()
{
    QString filepath = QFileDialog::getSaveFileName(this);
    if(filepath.isEmpty())
        return;

    m_view->save(filepath);
}

void MainWindow::onActionSaveAllTriggered()
{

}

void MainWindow::onFileOpened(QString filepath)
{
    JZProjectItem *item = m_project.getItem(filepath);
    m_editor->setFile(item);
}