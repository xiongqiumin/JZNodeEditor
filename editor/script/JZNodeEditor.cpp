#include "JZNodeEditor.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>

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
    connect(m_view,&JZNodeView::redoAvailable,this,&JZNodeEditor::redoAvailable);
    connect(m_view,&JZNodeView::undoAvailable,this,&JZNodeEditor::undoAvailable);

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    this->setLayout(l);    
    
    QSplitter *splitter = new QSplitter(Qt::Horizontal);    
    splitter->addWidget(m_nodePanel);
    splitter->addWidget(m_view);
    splitter->addWidget(m_nodeProp);

    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);
    splitter->setCollapsible(2,false);
    splitter->setStretchFactor(0,0);
    splitter->setStretchFactor(1,1);
    splitter->setStretchFactor(2,0);
    splitter->setSizes({150,300,150});
    l->addWidget(splitter);
    
    //m_nodeProp->setMaximumWidth(200);
    m_view->setPropertyEditor(m_nodeProp);
}

bool JZNodeEditor::isFirstShow(JZScriptFile* file)
{
    auto list = file->nodeList();
    for(int i = 0; i < list.size(); i++)
    {
        if(!file->getNodePos(list[i]).isNull())
            return false;
    }
    return true;
}

void JZNodeEditor::open(JZProjectItem *item)
{
    JZScriptFile* file = dynamic_cast<JZScriptFile*>(item);
    m_view->setFile(file);    
    m_nodePanel->setFile(file);

    if(isFirstShow(file))
        m_view->updateNodeLayout();
}

void JZNodeEditor::close()
{

}

void JZNodeEditor::save()
{
    m_view->syncNodePos();
}

void JZNodeEditor::updateMenuBar(QMenuBar *menubar)
{

}

bool JZNodeEditor::isModified()
{
    return true;
}

void JZNodeEditor::undo()
{
    m_view->undo();
}

void JZNodeEditor::redo()
{
    m_view->redo();
}

void JZNodeEditor::remove()
{
    m_view->remove();
}

void JZNodeEditor::cut()
{
    m_view->cut();
}

void JZNodeEditor::copy()
{
    m_view->copy();
}

void JZNodeEditor::paste()
{
    m_view->paste();
}

void JZNodeEditor::selectAll()
{
    m_view->selectAll();
}

void JZNodeEditor::updateNodeLayout()
{
    m_view->updateNodeLayout();
}
