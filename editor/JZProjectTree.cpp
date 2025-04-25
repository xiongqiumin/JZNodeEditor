#include "JZProjectTree.h"
#include <QVBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QShortcut>
#include <QLineEdit>
#include <QFileDialog>
#include <QKeyEvent>
#include <JZRegExpHelp.h>
#include <QDebug>
#include "JZNodeFuctionEditDialog.h"
#include "JZNewFileDialog.h"
#include "JZNodeClassEditDialog.h"
#include "JZUiItem.h"
#include "JZProjectSettingDialog.h"
#include "JZNodeSlotEditDialog.h"
#include "JZEditorGlobal.h"


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

    auto root = addItem(m_tree->invisibleRootItem(), m_project->root());
    root->setText(0, m_project->name());
    sortItem(root);

    m_tree->expandAll();
}

QTreeWidgetItem* JZProjectTree::addItem(QTreeWidgetItem *parent, JZProjectItem *item)
{
    Q_ASSERT(getProjectItem(parent) == item->parent());

    QTreeWidgetItem *view_item = new QTreeWidgetItem();
    m_itemMap[item] = view_item;

    view_item->setText(0,item->name());    
    parent->addChild(view_item);
    setItem(view_item, item);

    return view_item;
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
    JZProjectItem *item = getProjectItem(view_item);
    if (item == m_project->mainFunction() || item == m_project->mainFile()
        || item->itemType() == ProjectItem_ui || item->itemType() == ProjectItem_param)
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

void JZProjectTree::sortItem(QTreeWidgetItem *item)
{
    if (item->childCount() == 0)
        return;

    for (int i = 0; i < item->childCount(); i++)
        sortItem(item->child(i));

    UiHelper::treeSortChilds(item, [this](QTreeWidgetItem *a, QTreeWidgetItem *b)->bool {
        int a_pri = getProjectItem(a)->itemType();
        int b_pri = getProjectItem(b)->itemType();        
        if (a_pri != b_pri)
            return a_pri < b_pri;
        else
            return a->text(0) < b->text(0);
    });
}

void JZProjectTree::setItem(QTreeWidgetItem *view_item,JZProjectItem *item)
{
    QString icon_path;
    if (item->name() == ".")
        icon_path = ":/JZNodeEditor/Resources/icons/iconProject.png";
    else if (item->itemType() == ProjectItem_folder)
        icon_path = ":/JZNodeEditor/Resources/icons/iconFolder.png";    
    else if (item->itemType() == ProjectItem_class)
        icon_path = ":/JZNodeEditor/Resources/icons/iconClass.png";
    else if (item->itemType() == ProjectItem_scriptItem)
        icon_path = ":/JZNodeEditor/Resources/icons/iconFunction.png";
    else if (item->itemType() == ProjectItem_ui)
        icon_path = ":/JZNodeEditor/Resources/icons/iconUi.png";
    else
        icon_path = ":/JZNodeEditor/Resources/icons/iconFile.png";

    view_item->setIcon(0, QIcon(icon_path));    

    auto list = item->childs();
    for(int i = 0; i < list.size(); i++)
    {   
        JZProjectItem *sub_item = list[i];
        addItem(sub_item);
    }
}

bool JZProjectTree::canOpenItem(JZProjectItem *item)
{
    if (item->itemType() == ProjectItem_ui
        || item->itemType() == ProjectItem_param
        || item->itemType() == ProjectItem_scriptItem)
        return true;

    return false;
}

QTreeWidgetItem *JZProjectTree::getItem(QString path)
{
    JZProjectItem *proj_item = m_project->getItem(path);
    if (!proj_item)
        return nullptr;

    return getViewItem(proj_item);
}

QTreeWidgetItem *JZProjectTree::getViewItem(JZProjectItem *proj_item)
{
    return m_itemMap.value(proj_item);
}

JZProjectItem *JZProjectTree::getProjectItem(QTreeWidgetItem *view_item)
{
    return m_itemMap.key(view_item);
}

void JZProjectTree::cancelEdit()
{
    m_tree->blockSignals(true);
    if(m_editItem)
    {
        QString old_name = getProjectItem(m_editItem)->name();

        m_editItem->setFlags(m_editItem->flags() & ~Qt::ItemIsEditable);
        m_editItem->setText(0, old_name);        
        m_editItem = nullptr;        
    }
    m_tree->blockSignals(false);
}

QString JZProjectTree::filepath(QTreeWidgetItem *item)
{
    return getProjectItem(item)->itemPath();
}

bool JZProjectTree::dealRenameItem(JZProjectItem *item,QString name)
{
    if(!m_project->renameItem(item,name))
    {
        QMessageBox::information(this,"","重命名失败");
        return false;
    }

    sortItem(getViewItem(item)->parent());
    return true;
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

    QString old_name = getProjectItem(m_editItem)->name();    
    QString name = m_editItem->text(0);
    if (old_name == name)
        return;

    auto item_parent = m_editItem->parent();
    auto p = getProjectItem(item_parent);
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
        bool pre_select = (m_tree->currentItem() == item);
        auto project_item = getProjectItem(m_editItem);
        if(!dealRenameItem(project_item,name))        
            return;        

        if(pre_select)
            m_tree->setCurrentItem(item);
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
    JZProjectItem *item = getProjectItem(view_item);
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

    JZProjectItem *item = getProjectItem(view_item);
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
    const JZNodeObjectDefine *meta = nullptr;
    if(item_class)
        meta = editorObjectManager()->meta(item_class->className());

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
    if (canOpenItem(item))
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
        dlg.init(m_project->path());
        if (dlg.exec() == QDialog::Accepted)
        {
            QString path = dlg.path();
            QString name = dlg.name();

            JZScriptFile *new_item = new JZScriptFile();
            new_item->setName(name + ".jz");
            
            QString file_path = path + "/" + name + ".jz";
            if (QFile::exists(file_path))
            {
                QMessageBox::information(this, "", "文件已存在");
                return;
            }
            if (!m_project->addItem(path, new_item))
            {
                QMessageBox::information(this, "", "添加项目失败");
                return;
            }
            if (dlg.type() == JZNewFileDialog::NewClass)
            {                
                new_item->addClass(name);
            }
            else if (dlg.type() == JZNewFileDialog::NewUiClass)
            {
                JZScriptClassItem *class_item = new_item->addClass(name,"QWidget");
                class_item->addUi(new JZUiItem());
            }
            
            addItem(new_item);
            m_project->saveItem(new_item);            
        }
    }
    else if (act == actExistFile)
    {
        QStringList filelist = QFileDialog::getOpenFileNames(this, "", QString(), "All(*.jz)");
        for (int i = 0; i < filelist.size(); i++)
        {
            auto new_item = m_project->addFile(filelist[i]);
            if (!new_item) {
                QMessageBox::information(this, "", "添加" + filelist[i] + "失败");
                return;
            }

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
            dialog.init(m_project);
            if (dialog.exec() != QDialog::Accepted)
                return;

            function = dialog.functionInfo();
        }
        else if (act == actSlot)
        {
            JZNodeSlotEditDialog dlg(this);
            dlg.setClass(meta);
            if (dlg.exec() != QDialog::Accepted)
                return;

            function = meta->initSlotFunction(dlg.param(), dlg.signal());
        }
        else
        {
            function = meta->initVirtualFunction(act->text());
        }

        JZScriptItem *func_item = new JZScriptItem(JZScriptItem::Function);
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
        
        auto file_item = dynamic_cast<JZScriptFile*>(item);
        auto class_item = file_item->addClass(def, super);
        addItem(view_item, class_item);
        if (dialog.isUi())
            class_item->addUi(new JZUiItem());

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
        if (item->itemType() == ProjectItem_scriptItem)
        {
            JZScriptItem *func_item = dynamic_cast<JZScriptItem*>(item);
            QString oldName = func_item->name();

            JZNodeFuctionEditDialog dialog(this);
            dialog.setFunctionInfo(func_item->function(), false);
            dialog.init(m_project);
            if (dialog.exec() != QDialog::Accepted)
                return;
            
            JZFunctionDefine def = dialog.functionInfo();             
            if (oldName != def.name)
            {                
                if(!m_project->renameItem(func_item, def.name))
                {
                    QMessageBox::information(this,"","重命名失败");
                    return;
                }
                view_item->setText(0, def.name);
            }
            func_item->setFunction(def);
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
            if (dlg.isUi() && class_item->hasUi())
                class_item->addUi(new JZUiItem());
            else
                class_item->removeUi();
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