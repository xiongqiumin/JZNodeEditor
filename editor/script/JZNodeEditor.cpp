#include "JZNodeEditor.h"
#include <QVBoxLayout>
#include <QSplitter>

JZNodeEditor::JZNodeEditor()
{
    init();
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

}