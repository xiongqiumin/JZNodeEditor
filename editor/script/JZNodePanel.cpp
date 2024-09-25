#include <QVBoxLayout>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "UiCommon.h"
#include "JZNodePanel.h"
#include "JZNodeExpression.h"
#include "JZRegExpHelp.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeValue.h"
#include "JZNodeFactory.h"
#include "JZProjectItem.h"
#include "JZNodeObject.h"
#include "JZProject.h"
#include "JZScriptItem.h"
#include "JZNodeFunction.h"
#include "JZNodeEvent.h"
#include "JZNodeLocalParamEditDialog.h"
#include "JZNodeEditorManager.h"
#include "JZNodeView.h"

// JZNodeTreeWidget
QMimeData *JZNodeTreeWidget::mimeData(const QList<QTreeWidgetItem *> items) const
{
    Q_ASSERT(items.size() == 1);
    auto item = items[0];
    if(item->data(0,TreeItem_type).isNull())
        return nullptr;

    QMimeData *mimeData = new QMimeData();
    QString name = item->data(0, TreeItem_type).toString();
    mimeData->setData(name, item->data(0, TreeItem_value).toByteArray());
    return mimeData;
}

// JZNodePanel
JZNodePanel::JZNodePanel(QWidget *widget)
    : QWidget(widget)
{    
    m_file = nullptr;
    m_view = nullptr;
    m_classFile = nullptr;    
    m_memberFunction = nullptr;
    m_itemMemberParam = nullptr;
    m_itemInputParam = nullptr;
    m_itemLocalParam = nullptr;
    m_itemGlobalParam = nullptr;

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(1);

    m_lineSearch = new QLineEdit();
    layout->addWidget(m_lineSearch);
    connect(m_lineSearch,&QLineEdit::returnPressed,this,&JZNodePanel::onSearch);

    m_tree = new JZNodeTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);
    m_tree->setDragEnabled(true);   

    connect(m_tree, &QTreeWidget::itemClicked, this, &JZNodePanel::onTreeItemClicked);    
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &JZNodePanel::onContextMenu);

    layout->addWidget(m_tree);
    setLayout(layout);

    QFile file(":/JZNodeEditor/Resources/moduleList.json");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        auto toStringList = [](const QJsonValue &v)->QStringList {
            QStringList ret;
            if (v.isArray())
            {
                auto list = v.toArray();
                for (int i = 0; i < list.size(); i++)
                {
                    ret << list[i].toString();
                }
            }
            return ret;
        };

        QByteArray buffer = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(buffer);
        auto list = doc.array();
        for (int i = 0; i < list.size(); i++)
        {
            auto obj = list[i].toObject();
            QString name = obj["name"].toString();
            QStringList functionList = toStringList(obj["function"]);
            QStringList classList = toStringList(obj["class"]);
            QStringList depends = toStringList(obj["depend"]);

            JZModuleStatic m;
            m.init(name, classList, functionList, depends);
            m_modules.push_back(m);
        }
    }
}

JZNodePanel::~JZNodePanel()
{
}

void JZNodePanel::setFile(JZScriptItem *file)
{
    m_file = file;    
    m_classFile = m_file->project()->getItemClass(m_file);
    init();
}

void JZNodePanel::setView(JZNodeView *view)
{
    m_view = view;
}

void JZNodePanel::updateDefine()
{    
    updateThis();
    updateInputVariable();
    updateLocalVariable();
    updateGlobalVariable();
    updateModule();    
}

void JZNodePanel::updateThis()
{
    if (!m_classFile)
        return;

    auto def = m_classFile->objectDefine();
    //function    
    for (int i = 0; i < def.functions.size(); i++)
    {        
        auto item = createFunction(def.functions[i].fullName());
        m_memberFunction->addChild(item);
    }

    //params    
    QStringList params;
    params << "this" << def.paramList(false);
    for(int i = 1; i < params.size(); i++)
        params[i] = "this." + params[i];
    
    updateVariable(m_itemMemberParam, params);
}

void JZNodePanel::updateVariable(QTreeWidgetItem *root, QStringList paramList)
{
    auto list = UiHelper::treeDiff(root, paramList);
    for (int i = 0; i < list.size(); i++)
    {
        auto ret = list[i];
        if (ret.type == TreeDiffResult::Remove)
        {
            removeItem(root, list[i].name);
        }
        else if (ret.type == TreeDiffResult::Add)
        {
            QTreeWidgetItem *item = createParam(list[i].name);
            root->addChild(item);
        }
    }
}

void JZNodePanel::updateInputVariable()
{
    auto in_list = m_file->function().paramIn;
    QStringList params;
    for (int i = 0; i < in_list.size(); i++)
        params << in_list[i].name;
    if (m_file->function().isMemberFunction())
        params.pop_front();

    m_itemInputParam->setHidden(params.size() == 0);
    updateVariable(m_itemInputParam, params);
}

void JZNodePanel::updateLocalVariable()
{
    if (m_file->itemType() != ProjectItem_scriptFunction)
        return;

    QStringList params = m_file->localVariableList(false);
    updateVariable(m_itemLocalParam, params);        
}

void JZNodePanel::updateGlobalVariable()
{
    QStringList params = m_file->project()->globalVariableList();
    m_itemGlobalParam->setHidden(params.size() == 0);
    updateVariable(m_itemGlobalParam, params);
}

void JZNodePanel::updateModule()
{
    QStringList module_list;
    for(int md_idx = 0; md_idx < m_modules.size(); md_idx++)
        module_list << m_modules[md_idx].name();
    module_list << m_file->project()->moduleList();

    auto list = UiHelper::treeDiff(m_module,module_list);
    for(int i = 0; i < list.size(); i++)
    {
        auto ret = list[i];
        if(ret.type == TreeDiffResult::Remove)
        {            
            removeItem(m_module,list[i].name);
        }
        else
        {
            addModule(m_module,list[i].name);
        }
    }
}

void JZNodePanel::updateLocalDefine()
{
    QStringList tree_item_list;
    QList<QTreeWidgetItem*> tree_items;
    QStringList class_item_list;
    for (int i = 0; i < m_itemLocalDefine->childCount(); i++)
    {
        tree_item_list << m_itemLocalDefine->child(i)->text(0);
        tree_items << m_itemLocalDefine->child(i);
    }

    //update
    QStringList class_list = m_file->project()->classList();
    class_list << m_file->project()->containerList();
    for(int i = 0; i < class_list.size(); i++)
    {
        int index = tree_item_list.indexOf(class_list[i]);
        if(index == -1)        
            m_itemLocalDefine->addChild(createClass(class_list[i]));        
        else        
            updateClass(tree_items[index],class_list[i],false);                     
    }

    QStringList function_list = m_file->project()->functionList();    
    for(int i = 0; i < function_list.size(); i++)
    {        
        if(!tree_item_list.contains(function_list[i]))
        {
            QTreeWidgetItem *func_item = createFunction(function_list[i]);
            m_itemLocalDefine->addChild(func_item);
        }
    }

    //remove
    for (int i = 0; i < tree_item_list.size(); i++)    
    {
        if(!class_item_list.contains(tree_item_list[i]))
            delete tree_items[i];
    }
}

void JZNodePanel::init()
{
    m_tree->clear();    

    initData();
    initBasicFlow();

    m_module = createFolder("模块");
    m_tree->addTopLevelItem(m_module);

    updateDefine();
           
    for (int i = 0; i < m_tree->topLevelItemCount(); i++)
        m_tree->topLevelItem(i)->setExpanded(true);
}

QTreeWidgetItem *JZNodePanel::createFolder(QString name)
{
    QString icon_path = ":/JZNodeEditor/Resources/icons/iconFolder.png";

    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setIcon(0, QIcon(icon_path));

    item->setText(0, name);
    item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
    return item;
}

void JZNodePanel::setNode(QTreeWidgetItem *item,JZNode *node)
{
    item->setText(0, node->name());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0, TreeItem_type, "node_data");
    item->setData(0,TreeItem_value, JZNodeFactory::instance()->saveNode(node));
}

QTreeWidgetItem *JZNodePanel::createNode(JZNode *node)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    setNode(item,node);    
    return item;
}

QTreeWidgetItem *JZNodePanel::createParam(QString name)
{
    QStringList names = name.split(".");

    QString full_name = name;
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, names.back());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0,TreeItem_type,"node_param");    
    item->setData(0,TreeItem_value,full_name);

    return item;
}

QTreeWidgetItem *JZNodePanel::createMemberParam(QString name)
{
    QStringList names = name.split(".");

    QString full_name = name;
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, names.back());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0, TreeItem_type, "node_memberParam");
    item->setData(0, TreeItem_value, full_name);

    return item;
}

QTreeWidgetItem *JZNodePanel::createFunction(QString name)
{
    QTreeWidgetItem *item = nullptr;
    auto env = editorEnvironment();
    auto func_inst = editorFunctionManager();
    auto func_def = func_inst->function(name);
    int node_type = env->editorManager()->customFunctionNode(name);
    if(node_type != Node_none)
    { 
        auto node = JZNodeFactory::instance()->createNode(node_type);
        auto node_custom = dynamic_cast<JZNodeFunctionCustom*>(node);
        node_custom->setFunction(name);
        item = createNode(node_custom);
        delete node_custom;
    }
    else
    {
        JZNodeFunction func_node;
        func_node.setFunction(func_def);
        item = createNode(&func_node);        
    }

    item->setText(0, func_def->name);
    return item;
}

QTreeWidgetItem * JZNodePanel::createClass(QString class_name)
{
    QTreeWidgetItem *item_class = new QTreeWidgetItem();
    updateClass(item_class,class_name,false);
    return item_class;
}

void JZNodePanel::initData()
{
    if (m_classFile) {
        QTreeWidgetItem *itemClass = createFolder("类");        
        m_tree->addTopLevelItem(itemClass);
        initThis(itemClass);
    }

    QTreeWidgetItem *itemDataFolder = createFolder("数据");
    m_tree->addTopLevelItem(itemDataFolder);

    QTreeWidgetItem *itemConst = createFolder("常量");
    itemDataFolder->addChild(itemConst);
    initConstParam(itemConst);    
    
    m_itemInputParam = createFolder("输入参数");
    itemDataFolder->addChild(m_itemInputParam);
    
    initScriptParam(itemDataFolder);

    m_itemGlobalParam = createFolder("全局变量");
    itemDataFolder->addChild(m_itemGlobalParam);

    QTreeWidgetItem *itemOp = createFolder("操作");
    itemDataFolder->addChild(itemOp);    
    
    JZNodeParam node_param;
    itemOp->addChild(createNode(&node_param));
    if (m_file->itemType() == ProjectItem_scriptParamBinding)
    {
        //JZNodeParam node_param;
        //itemOp->addChild(createNode("set",Node_setParamData));
    }
    else
    {
        JZNodeSetParam node_setParam;
        JZNodeCreate node_create;
        JZNodeCreateFromString node_createFormString;
        itemOp->addChild(createNode(&node_setParam));
        itemOp->addChild(createNode(&node_create));
        itemOp->addChild(createNode(&node_createFormString));
    }   

    JZNodeMemberParam node_memberParam;
    JZNodeSetMemberParam node_setMemberParam;
    itemOp->addChild(createNode(&node_memberParam));
    itemOp->addChild(createNode(&node_setMemberParam));

    JZNodeSwap node_swap;
    JZNodeClone node_clone;
    itemOp->addChild(createNode(&node_swap));
    itemOp->addChild(createNode(&node_clone));

    JZNodeConvert node_convert;
    JZNodeDisplay node_display;
    JZNodePrint node_print;
    itemOp->addChild(createNode(&node_convert));
    itemOp->addChild(createNode(&node_display));
    itemOp->addChild(createNode(&node_print));
}

QTreeWidgetItem *JZNodePanel::itemOp()
{
    return m_itemOp;
}

QTreeWidgetItem *JZNodePanel::itemProcess()
{
    return m_itemProcess;
}

void JZNodePanel::initBasicFlow()
{
    QTreeWidgetItem *itemFlow = createFolder("语句");
    m_tree->addTopLevelItem(itemFlow);
    initProcess(itemFlow);           
    initExpression(itemFlow);
}

/*
void JZNodePanel::initEnums()
{
    QTreeWidgetItem *item_enum_root = createFolder("枚举");
    m_tree->addTopLevelItem(item_enum_root);

    auto enum_list = JZNodeObjectManager::instance()->getEnumList();
    enum_list.sort();
    for (int i = 0; i < enum_list.size(); i++)
    {        
        QTreeWidgetItem *item_enum = createNode(enum_list[i],Node_enum,{enum_list[i]});        
        item_enum_root->addChild(item_enum);        
    }
}
*/

void JZNodePanel::initLocalDefine()
{
    QTreeWidgetItem *item_local = createFolder("本地");
    m_tree->addTopLevelItem(item_local);
    m_itemLocalDefine = item_local;        
}

void JZNodePanel::addModule(QTreeWidgetItem *item_root,QString name)
{    
    auto func_inst = editorFunctionManager();
    const JZModule *m = module(name);
    auto item_module = createFolder(m->name());
    item_root->addChild(item_module);

    auto functionList = m->functionList();
    for (int func_idx = 0; func_idx < functionList.size(); func_idx++)
    {
        QString func_name = functionList[func_idx];
        auto *func = func_inst->function(func_name);
        Q_ASSERT_X(func,"Error Function",qUtf8Printable(func_name));
    
        auto function_node = createFunction(func->fullName());
        item_module->addChild(function_node);
    }

    auto classList = m->classList();
    for (int cls_idx = 0; cls_idx < classList.size(); cls_idx++)
    {   
        QTreeWidgetItem *item_class = createClass(classList[cls_idx]);
        item_module->addChild(item_class);
    }
}

void JZNodePanel::updateClass(QTreeWidgetItem *item_class,const QString &class_name,bool show_protected)
{
    auto obj_inst = editorObjectManager();
    auto enum_meta = obj_inst->enumMeta(class_name);
    if(enum_meta)
    {
        JZNodeEnum node_enum;
        node_enum.setEnum(enum_meta);
        setNode(item_class,&node_enum);

        item_class->setData(0, TreeItem_isClass, QVariant());
        return;
    }

    auto meta = obj_inst->meta(class_name);
    Q_ASSERT_X(meta,"Error Class",qUtf8Printable(class_name));

    item_class->setText(0, class_name);
    item_class->setData(0, TreeItem_isClass, true);

    if(meta->super())
    {
        auto item_super = createClass(meta->superName);
        item_class->addChild(item_super);
    }

    QStringList tree_item_list;
    QList<QTreeWidgetItem*> tree_items;
    QStringList class_item_list;
    for (int i = 0; i < item_class->childCount(); i++)    
    {
        tree_item_list << item_class->child(i)->text(0);
        tree_items << item_class->child(i);
    }

    for (int i = 0; i < meta->enums.size(); i++)
    {                        
        if(!tree_item_list.contains(meta->enums[i]))
        {
            JZNodeEnum node_enum;
            node_enum.setEnum(obj_inst->enumMeta(meta->enums[i]));
            QTreeWidgetItem *item_enum = createNode(&node_enum);
            item_class->addChild(item_enum);
        }
        class_item_list << meta->enums[i];
    }

    auto it = meta->params.begin();
    while(it != meta->params.end())
    {        
        if(!tree_item_list.contains(it->name))
        {
            QTreeWidgetItem *item_param = createMemberParam(class_name + "." + it->name);
            item_class->addChild(item_param);
        }
        class_item_list << it->name;
        it++;
    }
    
    for (int func_idx = 0; func_idx < meta->functions.size(); func_idx++)
    {
        auto func = &meta->functions[func_idx];
        if(!show_protected && func->isProtected)
            continue;

        if(!tree_item_list.contains(func->name))
        {
            auto function_node = createFunction(func->fullName());
            item_class->addChild(function_node);
        }
        class_item_list << func->name;
    }           

    //remove
    for (int i = 0; i < tree_item_list.size(); i++)    
    {
        if(!class_item_list.contains(tree_item_list[i]))
            delete tree_items[i];
    }
}

void JZNodePanel::initThis(QTreeWidgetItem *root)
{       
    auto def = m_classFile->objectDefine();
    if(def.superName.isEmpty())
    {
        root->addChild(createClass(def.superName));
    }

    m_itemMemberParam = createFolder("成员变量");
    root->addChild(m_itemMemberParam);

    m_memberFunction = createFolder("成员函数");
    root->addChild(m_memberFunction);    
    
    QTreeWidgetItem *itemClassEvent = createFolder("事件");
    root->addChild(itemClassEvent);
}

void JZNodePanel::initScriptParam(QTreeWidgetItem *root)
{    
    QPushButton *btn = new QPushButton("+");
    btn->setMaximumWidth(24);
    connect(btn, &QPushButton::clicked, this, &JZNodePanel::onAddScriptParam);    

    QWidget *w = new QWidget();
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);    
    l->addWidget(new QLabel("局部变量"));
    l->addStretch();
    l->addWidget(btn);
    w->setLayout(l);    

    m_itemLocalParam = createFolder("");
    root->addChild(m_itemLocalParam);
    m_tree->setItemWidget(m_itemLocalParam,0,w);    
}

void JZNodePanel::initConstParam(QTreeWidgetItem *root)
{
    JZNodeLiteral node_bool;
    JZNodeLiteral node_int;
    JZNodeLiteral node_int64;
    JZNodeLiteral node_double;
    JZNodeLiteral node_string;
    JZNodeLiteral node_null;

    node_bool.setDataType(Type_bool);
    node_int.setDataType(Type_int);
    node_int64.setDataType(Type_int64);
    node_double.setDataType(Type_double);
    node_string.setDataType(Type_string);
    node_null.setDataType(Type_nullptr);

    root->addChild(createNode(&node_bool));    
    root->addChild(createNode(&node_int));
    root->addChild(createNode(&node_int64));
    root->addChild(createNode(&node_double));
    root->addChild(createNode(&node_string));
    root->addChild(createNode(&node_null));

    JZNodeFunctionPointer node_func;
    root->addChild(createNode(&node_func));
}

void JZNodePanel::initConvert(QTreeWidgetItem *root)
{    
}

void JZNodePanel::initExpression(QTreeWidgetItem *root)
{        
    QTreeWidgetItem *itemExpr = createFolder("运算符");
    m_itemOp = itemExpr;    

    for (int i = Node_add; i <= Node_expr; i++)
    {   
        auto node = JZNodeFactory::instance()->createNode(i);
        QTreeWidgetItem *sub = createNode(node);
        itemExpr->addChild(sub);
        delete node;
    }
    root->addChild(itemExpr);
}

void JZNodePanel::initProcess(QTreeWidgetItem *root)
{
    QTreeWidgetItem *item_process = createFolder("过程");    ;
    m_itemProcess = item_process;

    JZNodeBranch node_branch;
    JZNodeIf node_if;
    JZNodeSwitch node_switch;
    JZNodeSequence node_sequence;
    JZNodeWhile node_while;
    JZNodeFor node_for;
    JZNodeForEach node_foreach;
    JZNodeContinue node_continue;
    JZNodeBreak node_break;    
    JZNodeNop node_nop;
    JZNodeReturn node_return;
    node_return.setFunction(&m_file->function());
    JZNodeExit node_exit;   

    item_process->addChild(createNode(&node_branch));
    item_process->addChild(createNode(&node_if));
    item_process->addChild(createNode(&node_switch));
    item_process->addChild(createNode(&node_sequence));
    
    item_process->addChild(createNode(&node_while));
    item_process->addChild(createNode(&node_for));
    item_process->addChild(createNode(&node_foreach));
    item_process->addChild(createNode(&node_continue));
    item_process->addChild(createNode(&node_break));    

    item_process->addChild(createNode(&node_nop));

    item_process->addChild(createNode(&node_return));
    item_process->addChild(createNode(&node_exit));

    JZNodeSignalConnect node_connect;
    item_process->addChild(createNode(&node_connect));

    root->addChild(item_process);
}

bool JZNodePanel::isClassItem(QTreeWidgetItem *item)
{
    auto flag = item->data(0, TreeItem_isClass);
    if (!flag.isValid())
        return false;

    return flag.toBool();
}

const JZModule *JZNodePanel::module(QString name)
{
    for (int i = 0; i < m_modules.size(); i++)
    {
        if (name == m_modules[i].name())
            return &m_modules[i];
    }

    return JZModuleManager::instance()->module(name);
}

QTreeWidgetItem *JZNodePanel::localVariableItem(QString name)
{
    int index = UiHelper::treeIndexOf(m_itemLocalParam, name);
    if (index == -1)
        return nullptr;

    return m_itemLocalParam->child(index);
}

void JZNodePanel::removeItem(QTreeWidgetItem *root, QString name)
{
    int index = UiHelper::treeIndexOf(root, name);
    if (index >= 0)
        root->removeChild(root->child(index));
}

bool JZNodePanel::filterItem(QTreeWidgetItem *item,QString name)
{
    bool show = false;
    int count = item->childCount();
    bool has_name = item->text(0).contains(name,Qt::CaseInsensitive);

    if(count == 0)
    {
        show = has_name;
    }
    else
    {
        if(isClassItem(item) && has_name)
        {
            show = true;
            for (int i = 0; i < count; i++)
                filterItem(item->child(i), QString());
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                if (filterItem(item->child(i), name))
                    show = true;
            }
        }
    }
    item->setHidden(!show);
    return show;
}

void JZNodePanel::onSearch()
{
    QString name = m_lineSearch->text();
    filterItem(m_tree->invisibleRootItem(),name);
    m_tree->expandAll();
}

void JZNodePanel::onTreeItemClicked(QTreeWidgetItem *item, int column)
{    
        
}

void JZNodePanel::onContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_tree->itemAt(pos);
    if (!item)
        return;

    QMenu menu;
    if (item->parent() != m_itemLocalParam)
        return;
       
    auto actEdit = menu.addAction("编辑");
    auto actDel = menu.addAction("删除");

    auto act = menu.exec(m_tree->mapToGlobal(pos));
    if (!act)
        return;

    if (act == actEdit)
    {
        auto old_def = m_file->localVariable(item->text(0));
        Q_ASSERT(old_def);

        JZNodeLocalParamEditDialog dialog(this);;
        dialog.setParam(*old_def);
        if (dialog.exec() != QDialog::Accepted)
            return;

        auto new_def = dialog.param();
        m_view->changeLocalVariableCommand(old_def->name, new_def);               
    }
    else if (act == actDel)
    {
        m_view->removeLocalVariableCommand(item->text(0));        
    }
}

void JZNodePanel::onAddScriptParam()
{
    JZParamDefine define;
    define.name = JZRegExpHelp::uniqueString("localVar", m_file->localVariableList(true));
    define.type = "int";

    JZNodeLocalParamEditDialog dialog(this);
    dialog.setParam(define);
    if (dialog.exec() != QDialog::Accepted)
        return;
    
    define = dialog.param();
    m_view->addLocalVariableCommand(define);       
}

void JZNodePanel::addLocalVariable(JZParamDefine define)
{
    m_file->addLocalVariable(define);
    QTreeWidgetItem *item = createParam(define.name);
    m_itemLocalParam->addChild(item);
    m_itemLocalParam->setExpanded(true);
}

void JZNodePanel::removeLocalVariable(QString name)
{
    auto item = localVariableItem(name);
    if (item)
    {
        m_file->removeLocalVariable(name);
        m_itemLocalParam->removeChild(item);
    }
}

void JZNodePanel::changeLocalVariable(QString name, JZParamDefine new_def)
{
    auto item = localVariableItem(name);
    if (item)
    {
        m_file->setLocalVariable(name, new_def);
        item->setText(0, new_def.name);
    }
}