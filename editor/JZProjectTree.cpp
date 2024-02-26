#include "JZProjectTree.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QShortcut>
#include <QLineEdit>
#include "JZNewClassDialog.h"
#include "JZNodeFuctionEditDialog.h"

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

void JZProjectTree::clear()
{
    m_project = nullptr;
    m_tree->clear();
}

void JZProjectTree::init()
{
    m_tree->clear();
    if(!m_project)
        return;

    QTreeWidgetItem *root = new QTreeWidgetItem();
    root->setText(0,m_project->name());
    m_tree->addTopLevelItem(root);
    setItem(root,m_project->root());
    m_tree->expandAll();
}

void JZProjectTree::addItem(QTreeWidgetItem *parent, JZProjectItem *item)
{
    Q_ASSERT(getFile(parent) == item->parent());

    QTreeWidgetItem *view_item = new QTreeWidgetItem();
    view_item->setText(0,item->name());
    view_item->setData(0, Qt::UserRole, item->name());
    parent->addChild(view_item);
    setItem(view_item, item);
}

void JZProjectTree::setItem(QTreeWidgetItem *view_item,JZProjectItem *item)
{
    QString icon_path;
    if (item->name() == ".")
        icon_path = ":/JZNodeEditor/Resources/icons/iconProject.png";
    else if (item->itemType() == ProjectItem_folder)
        icon_path = ":/JZNodeEditor/Resources/icons/iconFolder.png";
    else if (item->itemType() == ProjectItem_library)
        icon_path = ":/JZNodeEditor/Resources/icons/iconLibrary.png";
    else if (item->itemType() == ProjectItem_class)
        icon_path = ":/JZNodeEditor/Resources/icons/iconClass.png";
    else if (item->itemType() == ProjectItem_scriptFunction)
        icon_path = ":/JZNodeEditor/Resources/icons/iconFunction.png";
    else
        icon_path = ":/JZNodeEditor/Resources/icons/iconFile.png";

    view_item->setIcon(0, QIcon(icon_path));

    auto list = item->childs();
    for(int i = 0; i < list.size(); i++)
    {   
        JZProjectItem *sub_item = list[i];
        QTreeWidgetItem *sub_view = new QTreeWidgetItem();
        sub_view->setText(0,sub_item->name());        
        sub_view->setData(0, Qt::UserRole, sub_item->name());
        view_item->addChild(sub_view);
        setItem(sub_view,sub_item);
    }
}

QString JZProjectTree::getClass(JZProjectItem *item)
{    
    while (item)
    {
        if (item->itemType() == ProjectItem_class)
            return ((JZScriptClassItem*)item)->className();

        item = item->parent();
    }
    return QString();
}

QTreeWidgetItem *JZProjectTree::getItem(QString path)
{
    QStringList list = path.split("/");
    Q_ASSERT(list[0] == ".");

    QTreeWidgetItem *item = m_tree->topLevelItem(0);
    for (int level = 1; level < list.size(); level++)
    {
        item = nullptr;
        for (int i = 0; i < item->childCount(); i++)
        {
            auto child = item->child(i);
            if (child->text(0) == list[i])
            {
                item = child;
                break;
            }
        }
        if (!item)
            return nullptr;
    }
    return item;
}

JZProjectItem *JZProjectTree::getFile(QTreeWidgetItem *view_item)
{
    QString path = filepath(view_item);
    auto item = m_project->getItem(path);
    Q_ASSERT(item);
    return item;
}

void JZProjectTree::cancelEdit()
{
    m_tree->blockSignals(true);
    if(m_editItem)
    {
        QString old_name = getFile(m_editItem)->name();

        m_editItem->setFlags(m_editItem->flags() & ~Qt::ItemIsEditable);
        m_editItem->setText(0, old_name);        
        m_editItem = nullptr;        
    }
    m_tree->blockSignals(false);
}

QString JZProjectTree::filepath(QTreeWidgetItem *item)
{
    QTreeWidgetItem *root = m_tree->topLevelItem(0);
    QString path;
    while (item != root)
    {
        if (!path.isEmpty())
            path = "/" + path;;
        path = item->data(0,Qt::UserRole).toString() + path;
        item = item->parent();
    }
    return "./" + path;
}

void JZProjectTree::renameItem(QTreeWidgetItem *view_item)
{
    m_tree->scrollToItem(view_item);

    m_editItem = nullptr;
    view_item->setFlags(view_item->flags() | Qt::ItemIsEditable);
    m_editItem = view_item;    
    m_tree->editItem(view_item);
}

void JZProjectTree::onItemChanged(QTreeWidgetItem *item)
{
    if(m_editItem != item)
        return;    

    QString old_name = getFile(m_editItem)->name();    
    QString name = m_editItem->text(0);
    if (old_name == name)
        return;

    auto item_parent = m_editItem->parent();
    auto p = getFile(item_parent);
    QString name_error;
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
        auto project_item = getFile(m_editItem);
        project_item->setName(name);
        
        m_project->renameItem(project_item,name);
        item->setData(0,Qt::UserRole, project_item->name());

        int old_idx = item_parent->indexOfChild(item);
        int new_idx = project_item->parent()->indexOfItem(project_item);
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
        m_editItem->setText(0, old_name);
    }
    m_tree->blockSignals(false);    
    m_editItem = nullptr;    
}

void JZProjectTree::onItemClicked(QTreeWidgetItem *view_item)
{

}


void JZProjectTree::onItemDoubleClicked(QTreeWidgetItem *view_item)
{
    JZProjectItem *item = getFile(view_item);
    sigFileOpened(item->itemPath());
}

void JZProjectTree::onItemRename()
{
    auto view_item = m_tree->currentItem();
    if(!view_item || !view_item->parent())
        return;
    renameItem(view_item);    
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

    JZProjectItem *item = getFile(view_item);
    QMenu menu(this);            
    QAction *actRemove = nullptr;
    QAction *actRename = nullptr;    
    QAction *actCreateFunction = nullptr;
    QAction *actCreateClass = nullptr;
    QAction *actCreateEvent = nullptr;
    QAction *actEditFunction = nullptr;
    
    QString className = getClass(item);    
    if(item->itemType() == ProjectItem_folder
            || item->itemType() == ProjectItem_class)
    {        
        QMenu *menu_add = menu.addMenu("添加");
        QString funcAct = className .isEmpty()?  "函数" : "成员函数";
        actCreateFunction = menu_add->addAction(funcAct);
        if(className.isEmpty())
            actCreateClass = menu_add->addAction("类");
        actCreateEvent = menu_add->addAction("事件处理");
    }
    else if (item->itemType() == ProjectItem_scriptFunction)
    {
        actEditFunction = menu.addAction("编辑");
    }

    if(view_item->parent())
    {
        actRemove = menu.addAction("删除");
        actRename = menu.addAction("重命名");
    }

    if (item->itemPath() == m_project->mainScriptPath())
    {
        actRemove->setEnabled(false);
        actRename->setEnabled(false);        
    }

    if (menu.actions().size() > 0)
        menu.addSeparator();
    auto actProp = menu.addAction("属性");

    QAction *act = menu.exec(m_tree->mapToGlobal(pos));
    if(!act)
        return;

    if(act == actCreateFunction)
    {        
        FunctionDefine function;
        function.name = "new";
        if (!className.isEmpty())
        {
            function.className = className;

            int classType = JZNodeObjectManager::instance()->getClassId(className);
            JZParamDefine def;
            def.name = "this";
            def.dataType = classType;            
            function.paramIn.push_back(def);
        }

        JZNodeFuctionEditDialog dialog(this);
        dialog.setFunctionInfo(function,true);
        dialog.init();
        if (dialog.exec() != QDialog::Accepted)
            return;

        QTreeWidgetItem *parent_item = view_item;
        if (item->itemType() == ProjectItem_class)
        {
            if(item->getItem("成员函数"))
                parent_item = getItem(item->itemPath() + "/成员函数");
        }
        /*
        auto func_item = m_project->addFunction(item->itemPath(), dialog.functionInfo());
        addItem(parent_item, func_item);
        */
    }
    else if (act == actCreateClass)
    {
        JZNewClassDialog dialog(this);
        if(dialog.exec() != QDialog::Accepted)
            return;

        QString def = dialog.className();
        QString super = dialog.super();
        bool isUi = dialog.isUi();
        JZScriptClassItem *class_item = nullptr;
        /*
        if(isUi)
            class_item = m_project->addUiClass(item->path(), def);
        else
            class_item = m_project->addClass(item->path(), def, super);
        addItem(view_item, class_item);
        */
    }
    else if (act == actEditFunction)
    {        
        JZScriptItem *func_item = (JZScriptItem*)item;
        QString oldName = func_item->name();

        JZNodeFuctionEditDialog dialog(this);        
        dialog.setFunctionInfo(func_item->function(),false);
        dialog.init();
        if (dialog.exec() != QDialog::Accepted)
            return;

        FunctionDefine def = dialog.functionInfo();        
        func_item->setFunction(def);        

        if (oldName != def.name)
        {
            m_project->renameItem(func_item, def.name);
            view_item->setText(0,def.name);
        }
    }
    else if(act == actCreateEvent)
    {
        QString name = "事件";
        /*
        JZProjectItemFolder *folder = m_project->addFolder(item->itemPath(), name);
        addItem(view_item, folder);
        */
    }
    else if(act == actRemove)
    {
        emit sigFileRemoved(item->itemPath());
        delete view_item;
    }
    else if(act == actRename)
    {
        onItemRename();
    }
    else if (act == actProp)
    {
        QMessageBox::information(this, "", item->name());
    }
}