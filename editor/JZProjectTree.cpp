#include "JZProjectTree.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QShortcut>
#include <QLineEdit>
#include <QFileDialog>
#include <QKeyEvent>
#include <JZRegExpHelp.h>
#include "JZNodeFuctionEditDialog.h"
#include "JZNewFileDialog.h"
#include "JZNodeClassEditDialog.h"
#include "JZUIFile.h"
#include "JZProjectSettingDialog.h"
#include "JZNodeSlotEditDialog.h"

enum{
    Item_none,
};

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
    
    m_tree->setExpandsOnDoubleClick(false);
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
    m_tree->clear();
    m_project = nullptr;
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

void JZProjectTree::keyPressEvent(QKeyEvent *e)
{    
    if (e->key() == Qt::Key_Return)
    {
        auto item = m_tree->currentItem();
        if (item)
        {
            onItemDoubleClicked(item);
            e->accept();
            return;
        }
    }
    QWidget::keyPressEvent(e);
}

bool JZProjectTree::canItemRename(QTreeWidgetItem *view_item)
{
    JZProjectItem *item = getFile(view_item);
    if (item == m_project->mainFunction() || item->itemType() == ProjectItem_param)
        return false;

    return true;
}

void JZProjectTree::addItem(JZProjectItem *item)
{
    auto parent_item = item->parent();
    while (parent_item)
    {
        auto parent_view_item = getItem(parent_item->itemPath());
        if (parent_view_item)
        {
            addItem(parent_view_item, item);
            return;
        }
        item = parent_item;
        parent_item = parent_item->parent();
    }
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

bool JZProjectTree::canOpenItem(JZProjectItem *item)
{
    if (item->itemType() == ProjectItem_ui
        || item->itemType() == ProjectItem_param
        || item->itemType() == ProjectItem_scriptFunction
        || item->itemType() == ProjectItem_scriptParamBinding)
        return true;

    return false;
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
        m_project->saveItem(project_item);
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
    if(canOpenItem(item))
        sigActionTrigged(Action_open,item->itemPath());
    else    
        view_item->setExpanded(!view_item->isExpanded());    
}

void JZProjectTree::onItemRename()
{
    auto view_item = m_tree->currentItem();
    if(!canItemRename(view_item))
        return;
    
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
    if (!view_item)
        return;    

    JZProjectItem *item = getFile(view_item);
    QMenu menu(this);
    QAction *actRemove = nullptr;
    QAction *actRename = nullptr;
    QAction *actCreateFunction = nullptr;
    QAction *actCreateClass = nullptr;
    QList<QAction*> actCreateVirtual;
    QAction *actOpen = nullptr;
    QAction *actBuild = nullptr, *actRebuild = nullptr, *actClearBuild = nullptr;
    QAction *actNewFile = nullptr, *actExistFile = nullptr;
    QAction *actSlot = nullptr;

    bool canChanged = true;
    auto item_class = m_project->getItemClass(item);
    JZNodeObjectDefine *meta = nullptr;
    if(item_class)
        meta = JZNodeObjectManager::instance()->meta(item_class->className());

    if(item->itemType() == ProjectItem_root)
    {
        actBuild = menu.addAction("编译");
        actRebuild = menu.addAction("重新编译");
        actClearBuild = menu.addAction("清理");
        menu.addSeparator();
        auto menu_new = menu.addMenu("添加");
        actNewFile = menu_new->addAction("新建项");
        actExistFile = menu_new->addAction("现有项");
        menu.addSeparator();
        auto menu_debug = menu.addMenu("调试");
        menu_debug->addAction("启动");
        menu_debug->addAction("启动并中断");
        menu.addSeparator();
    }
    else if (item->itemType() == ProjectItem_folder)
    {
        auto menu_new = menu.addMenu("添加");
        actNewFile = menu_new->addAction("新建项");
        menu.addSeparator();
    }
    else if (item->itemType() == ProjectItem_scriptFile)
    {
        QMenu *menu_new = menu.addMenu("添加");
        actCreateClass = menu_new->addAction("类");
        actCreateFunction = menu_new->addAction("全局函数");
    }
    else if (item->itemType() == ProjectItem_ui)
    {
        actOpen = menu.addAction("打开");
    }
    else if (item->itemType() == ProjectItem_class)
    {
        QMenu *menu_new = menu.addMenu("添加");
        actCreateFunction = menu_new->addAction("成员函数");
        
        auto virtual_list = meta->virtualFunctionList();
        if(virtual_list.size() > 0)
        {        
            QMenu *menu_virtual = menu_new->addMenu("虚函数");
            for(int i = 0; i < virtual_list.size(); i++)
                actCreateVirtual << menu_virtual->addAction(virtual_list[i]);
        }
        actSlot = menu_new->addAction("槽函数");
    }
    else if (item->itemType() == ProjectItem_param
        || item->itemType() == ProjectItem_scriptFunction
        || item->itemType() == ProjectItem_scriptParamBinding)
    {
        actOpen = menu.addAction("打开");
    }

    if(item->itemPath() == m_project->mainFilePath()
        ||item->itemPath() == m_project->mainFunctionPath()
        ||item->itemType() == ProjectItem_param)
    {
        canChanged = false;
    }
    if(view_item->parent() && canChanged)
    {
        actRemove = menu.addAction("删除");
        actRename = menu.addAction("重命名");
    }    

    if (menu.actions().size() > 0)
        menu.addSeparator();

    auto actProp = menu.addAction("属性");
    QAction *act = menu.exec(m_tree->mapToGlobal(pos));
    if(!act)
        return;

    if (act == actBuild)
        emit sigActionTrigged(Action_build, item->itemPath());
    else if (act == actRebuild)
        emit sigActionTrigged(Action_reBuild, item->itemPath());
    else if (act == actClearBuild)
        emit sigActionTrigged(Action_clearBuild, item->itemPath());
    else if (act == actOpen)
        emit sigActionTrigged(Action_open, item->itemPath());
    else if (act == actNewFile)
    {
        JZNewFileDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted)
        {
            QString path = dlg.path();
            QString name = dlg.name();

            JZProjectItem *new_item = nullptr;
            if (dlg.type() == "jz")
            {
                new_item = new JZScriptFile();
                new_item->setName(name + ".jz");                
            }
            else
            {
                new_item = new JZUiFile();
                new_item->setName(name + ".ui");                
            }

            if (new_item)
            {
                m_project->addItem(path, new_item);
                addItem(new_item);
                m_project->saveItem(new_item);
            }
        }
    }
    else if (act == actExistFile)
    {
        QStringList filelist = QFileDialog::getOpenFileNames(this, "", QString(), "All(*.ui *.jz)");
        for (int i = 0; i < filelist.size(); i++)
        {
            auto new_item = m_project->addFile(filelist[i]);
            addItem(new_item);            
        }
    }
    else if(act == actCreateFunction || act == actSlot || actCreateVirtual.contains(act))
    {        
        JZFunctionDefine function;
        if (act == actCreateFunction)
        {
            if (item_class)
            {
                function.className = item_class->className();
                function.name = JZRegExpHelp::uniqueString("newFunction", item_class->memberFunctionList());

                JZParamDefine def;
                def.name = "this";
                def.type = function.className;
                function.paramIn.push_back(def);
                function.isFlowFunction = true;
            }
            else            
                function.name = JZRegExpHelp::uniqueString("newFunction", m_project->functionList());

            JZNodeFuctionEditDialog dialog(this);
            dialog.setFunctionInfo(function,true);
            dialog.init();
            if (dialog.exec() != QDialog::Accepted)
                return;

            function = dialog.functionInfo();
        }
        else if (act == actSlot)
        {
            JZNodeSlotEditDialog dlg(this);
            dlg.setClass(item_class);
            if (dlg.exec() != QDialog::Accepted)
                return;

            function = meta->initSlotFunction(dlg.param(), dlg.signal());
        }
        else
        {
            function = meta->initVirtualFunction(act->text());
        }

        JZScriptItem *func_item = new JZScriptItem(ProjectItem_scriptFunction);
        func_item->setFunction(function);
        m_project->addItem(item->itemPath(), func_item);
        m_project->saveItem(item);
        
        addItem(view_item, func_item);        
    }
    else if (act == actCreateClass)
    {
        JZNodeClassEditDialog dialog(this);
        if(dialog.exec() != QDialog::Accepted)
            return;

        QString def = dialog.className();
        QString super = dialog.super();
        QString file = dialog.uiFile();
        
        auto file_item = dynamic_cast<JZScriptFile*>(item);
        auto class_item = file_item->addClass(def, super);
        addItem(view_item, class_item);
        class_item->setUiFile(file);
        m_project->saveItem(class_item);
    }    
    else if(act == actRemove)
    {
        if (QMessageBox::question(this, "", "是否删除", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            emit sigActionTrigged(Action_remove, item->itemPath());
            delete view_item;
        }
    }
    else if(act == actRename)
    {        
        onItemRename();
    }
    else if (act == actProp)
    {        
        if (item->itemType() == ProjectItem_scriptFunction)
        {
            JZScriptItem *func_item = dynamic_cast<JZScriptItem*>(item);
            QString oldName = func_item->name();

            JZNodeFuctionEditDialog dialog(this);
            dialog.setFunctionInfo(func_item->function(), false);
            dialog.init();
            if (dialog.exec() != QDialog::Accepted)
                return;
            
            JZFunctionDefine def = dialog.functionInfo(); 
            func_item->setFunction(def);
            if (oldName != def.name)
            {
                m_project->renameItem(func_item, def.name);
                view_item->setText(0, def.name);
            }
            m_project->saveItem(func_item);
        }
        else if (item->itemType() == ProjectItem_class)
        {
            JZScriptClassItem *class_item = (JZScriptClassItem*)item;
            
            JZNodeClassEditDialog dlg(this);
            dlg.setClass(class_item);
            if (dlg.exec() != QDialog::Accepted)
                return;
             
            class_item->setClass(dlg.className(), dlg.super());
            class_item->setUiFile(dlg.uiFile());
            m_project->saveItem(class_item);
        } 
        else if(item->itemType() == ProjectItem_root)
        {
            JZProjectSettingDialog dlg(this);
            dlg.setProject(m_project);
            if (dlg.exec() != QDialog::Accepted)
                return;

            m_project->save();
        }
        else
            QMessageBox::information(this, "", item->name());
    }
}