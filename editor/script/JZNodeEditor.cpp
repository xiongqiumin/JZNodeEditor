#include "JZNodeEditor.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QDebug>

JZNodeEditor::JZNodeEditor()
{
    m_type = Editor_script;
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
    connect(m_view,&JZNodeView::modifyChanged,this,&JZNodeEditor::modifyChanged);

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
    m_nodePanel->setPropertyEditor(m_nodeProp);
}

void JZNodeEditor::open(JZProjectItem *item)
{
    JZScriptFile* file = dynamic_cast<JZScriptFile*>(item);
    m_view->setFile(file);    
    m_nodePanel->setFile(file);
    m_view->setSceneRect(m_view->rect());
}

void JZNodeEditor::close()
{

}

void JZNodeEditor::save()
{    
    m_view->save();
}

void JZNodeEditor::addMenuBar(QMenuBar *menubar)
{
    QMenu *menu = menubar->actions()[Menu_View]->menu();
    m_actionList << menu->addSeparator();
    m_actionList << menu->addAction("自动布局");
    m_actionList << menu->addAction("显示全部");
    connect(m_actionList[1], &QAction::triggered, this, &JZNodeEditor::onActionLayout);
    connect(m_actionList[2], &QAction::triggered, this, &JZNodeEditor::onActionFitInView);
}

void JZNodeEditor::removeMenuBar(QMenuBar *menubar)
{
    QMenu *menu = menubar->actions()[Menu_View]->menu();
    for (int i = 0; i < m_actionList.size(); i++)
    {
        menu->removeAction(m_actionList[i]);
        delete m_actionList[i];
    }
    m_actionList.clear();
}

bool JZNodeEditor::isModified()
{
    return m_view->isModified();
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


void JZNodeEditor::onActionLayout()
{
    m_view->updateNodeLayout();
}

void JZNodeEditor::onActionFitInView()
{
    m_view->fitNodeView();
}

void JZNodeEditor::ensureNodeVisible(int nodeId)
{
    m_view->ensureNodeVisible(nodeId);
}

BreakPointTriggerResult JZNodeEditor::breakPointTrigger()
{
    return m_view->breakPointTrigger();
}

void JZNodeEditor::setRuntimeStatus(int status)
{
    m_view->setRuntimeStatus(status);
}

int JZNodeEditor::runtimeNode()
{
    return m_view->runtimeNode();
}

void JZNodeEditor::setRuntimeNode(int nodeId)
{
    m_view->setRuntimeNode(nodeId);
}