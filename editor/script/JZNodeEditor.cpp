#include "JZNodeEditor.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>

JZNodeEditor::JZNodeEditor()
{
    init();
    
    auto cutF5 = new QShortcut(QKeySequence("F5"),this);
    auto cutF9 = new QShortcut(QKeySequence("F9"),this);
    auto cutF10 = new QShortcut(QKeySequence("F10"),this);
    auto cutF11 = new QShortcut(QKeySequence("F11"),this);

    connect(cutF5,&QShortcut::activated,this,&JZNodeView::onRun);
    connect(cutF9,&QShortcut::activated,this,&JZNodeView::onBreakPoint);
    connect(cutF10,&QShortcut::activated,this,&JZNodeView::onStepOver);
    connect(cutF11,&QShortcut::activated,this,&JZNodeView::onStepIn);
}

JZNodeEditor::~JZNodeEditor()
{

}

void JZNodeEditor::init()
{
    m_view = new JZNodeView();
    m_nodePanel = new JZNodePanel();
    m_nodeProp = new JZNodePropertyEditor();
        
    QVBoxLayout *l = new QVBoxLayout();    
    this->setLayout(l);    
    
    QSplitter *splitter = new QSplitter(Qt::Horizontal);    
    splitter->addWidget(m_nodePanel);
    splitter->addWidget(m_view);
    splitter->addWidget(m_nodeProp);

    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);
    splitter->setCollapsible(2,false);
    splitter->setStretchFactor(0,1);
    splitter->setStretchFactor(1,3);
    splitter->setStretchFactor(2,1);
    l->addWidget(splitter);
    
    m_nodeProp->setMaximumWidth(200);
    m_view->setPropertyEditor(m_nodeProp);
}

void JZNodeEditor::open(JZProjectItem *item)
{
    JZScriptFile *file = dynamic_cast<JZScriptFile*>(item);
    m_view->setFile(file);
}

void JZNodeEditor::onRun()
{
        
}

void JZNodeEditor::onBreakPoint()
{
    
}

void JZNodeEditor::onStepOver()
{
    
}

void JZNodeEditor::onStepIn()
{

}