#include "JZProjectTree.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QShortcut>

JZProjectTree::JZProjectTree()
{
    m_project = nullptr;    

    QVBoxLayout *l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    setLayout(l);

    m_tree = new QTreeWidget();
    m_editItem = nullptr;
    m_tree->setHeaderHidden(true);
    l->addWidget(m_tree);

    void onItemClicked(QTreeWidgetItem *item);
    void onItemDoubleClicked(QTreeWidgetItem *item);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree,&QWidget::customContextMenuRequested,this,&JZProjectTree::onContextMenu);

    connect(m_tree,&QTreeWidget::itemChanged,this,&JZProjectTree::onItemChanged);
    connect(m_tree,&QTreeWidget::itemClicked,this,&JZProjectTree::onItemClicked);
    connect(m_tree,&QTreeWidget::itemDoubleClicked,this,&JZProjectTree::onItemDoubleClicked);
    connect(m_tree,&QTreeWidget::currentItemChanged,this,&JZProjectTree::onCurrentItemChanged);

    auto cutRename = new QShortcut(QKeySequence("F2"),this);
    connect(cutRename,&QShortcut::activated,this,&JZProjectTree::onItemRename);
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
    m_tree->clear();
    if(!m_project)
        return;

    QTreeWidgetItem *root = new QTreeWidgetItem();
    root->setText(0,m_project->name());
    m_tree->addTopLevelItem(root);
    addItem(root,m_project->root());
    m_tree->expandAll();
}

void JZProjectTree::addItem(QTreeWidgetItem *view_item,JZProjectItem *item)
{
    auto list = item->childs();
    for(int i = 0; i < list.size(); i++)
    {   
        JZProjectItem *sub_item = list[i];
        QTreeWidgetItem *sub_view = new QTreeWidgetItem();
        sub_view->setText(0,sub_item->name());
        sub_view->setData(0,Qt::UserRole,sub_item->itemPath());
        view_item->addChild(sub_view);

        addItem(sub_view,sub_item);
    }
}

JZProjectItem *JZProjectTree::getItem(QTreeWidgetItem *view_item)
{
    return m_project->getItem(view_item->data(0,Qt::UserRole).toString());
}

void JZProjectTree::cancelEdit()
{
    m_tree->blockSignals(true);
    if(m_editItem)
    {
        m_editItem->setFlags(m_editItem->flags() & ~Qt::ItemIsEditable);
        m_editItem->setText(0,m_editProjectItem->name());
        m_editProjectItem = nullptr;
        m_editItem = nullptr;        
    }
    m_tree->blockSignals(false);
}

void JZProjectTree::onItemChanged(QTreeWidgetItem *item)
{
    if(m_editItem != item)
        return;

    QString name_error;
    QString name = m_editItem->text(0);

    auto item_parent = m_editItem->parent();
    auto p = getItem(item_parent);
    if(name.isEmpty())
        name_error = "名称不能为空";
    else if(name.contains("/"))
        name_error = "无效名称";
    else if(p->getItem(name))
        name_error = "名称重复";

    m_tree->blockSignals(true);
    m_editItem->setFlags(m_editItem->flags() & ~Qt::ItemIsEditable);
    if(name_error.isEmpty())
    {
        m_editProjectItem->setName(name);
        int new_idx = -1;
        m_project->renameItem(m_editProjectItem,name);
        item->setData(0,Qt::UserRole,m_editProjectItem->itemPath());

        int old_idx = item_parent->indexOfChild(item);
        if(old_idx != new_idx)
        {
            bool pre_select = (m_tree->currentItem() == item);
            item_parent->takeChild(old_idx);
            item_parent->insertChild(new_idx,item);
            if(pre_select)
                m_tree->setCurrentItem(item);
        }
    }
    else
    {
        QMessageBox::information(this,"",name_error);
        m_editItem->setText(0,m_editProjectItem->name());
    }
    m_tree->blockSignals(false);

    m_editProjectItem = nullptr;
    m_editItem = nullptr;    
}

void JZProjectTree::onItemClicked(QTreeWidgetItem *view_item)
{

}


void JZProjectTree::onItemDoubleClicked(QTreeWidgetItem *view_item)
{
    JZProjectItem *item = getItem(view_item);
    sigFileOpened(item->itemPath());
}

void JZProjectTree::onItemRename()
{
    auto view_item = m_tree->currentItem();
    if(!view_item || !view_item->parent())
        return;

    view_item->setFlags(view_item->flags() | Qt::ItemIsEditable);
    m_editItem = view_item;
    m_editProjectItem = getItem(view_item);    
    m_tree->editItem(view_item);
}

void JZProjectTree::onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if(m_editItem)
        cancelEdit();
}

void JZProjectTree::onContextMenu(QPoint pos)
{    
    QTreeWidgetItem *view_item = m_tree->itemAt(pos);
    if(!view_item)
        return;

    JZProjectItem *item = getItem(view_item); 
    QMenu menu(this);        
    QAction *actRemove = nullptr;
    QAction *actRename = nullptr;
    QAction *actCreate = nullptr;
    QAction *actCreateFolder = nullptr;
/*
    if(item->isFolder())
    {
        QString name = JZProjectItemFactory::itemTypeName(item->itemType());
        actCreate = menu.addAction("新建" + name);
        actCreateFolder = menu.addAction("新建文件夹");
    }
    if(view_item->parent())
    {
        actRemove = menu.addAction("删除");
        actRename = menu.addAction("重命名");
    }
*/    
    QAction *act = menu.exec(m_tree->mapToGlobal(pos));
    if(!act)
        return;

    if(act == actCreate || act == actCreateFolder)
    {

    }
    else if(act == actRemove)
    {
        m_project->removeItem(item->itemPath());
        delete view_item;
    }
    else if(act == actRename)
    {
        onItemRename();
    }
}
