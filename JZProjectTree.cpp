#include "JZProjectTree.h"
#include <QVBoxLayout>
#include <QMenu>

JZProjectTree::JZProjectTree()
{
    m_project = nullptr;    

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    setLayout(l);
    
    m_tree = new QTreeWidget();    
    l->addWidget(m_tree);

    connect(m_tree,&QWidget::customContextMenuRequested,this,&JZProjectTree::onContextMenu);
}
    
JZProjectTree::~JZProjectTree()
{
    
}

void JZProjectTree::setProject(JZProject *project)
{
    m_project = project;
    init();
}

void JZProjectTree::init()
{
    addItem(m_tree->invisibleRootItem(),m_project->root())
}

void JZProjectTree::addItem(QTreeWidgetItem *view_item,JZProjectItem *item)
{
    auto list = item->items().size();
    for(int i = 0; i < item->items().size(); i++)
    {
        QTreeWidgetItem *sub_view = new QTreeWidgetItem();
        sub_view->setText(0,list[i]->name());
        view_item->addChild(sub_view);

        addItem(sub_view,list[i]);
    }
}

void JZProjectTree::onItemDoubleClicked(QTreeWidgetItem *view_item)
{
    JZProjectItem *item = nullptr;
    sigFileOpened(item->filePath());
}

void JZProjectTree::onContextMenu(QPoint pos)
{
    QString dir;
    JZProjectItem *item = nullptr;
    auto view_item = m_tree->getItem(pos);

    QMenu menu(this);        
    QAction *actRemove = nullptr,actCreate = nullptr,actCreateFolder = nullptr;

    QAction *act = menu.exec(m_tree->mapToGlobal(pos));
    if(!act)
        return;

    if(act == actCreate || act == actCreateFolder)
    {
        JZProjectItem *new_item = new JZProjectItem(item->itemType(), act == actCreateFolder);        
        m_project->addFile(dir,new_item);    
    }
    else if(act == actRemove)
    {
        m_project->removeFile(item->itemPath());
    }
}