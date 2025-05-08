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
#include "JZProjectItem.h"
#include "JZNodeObject.h"
#include "JZProject.h"
#include "JZScriptItem.h"
#include "JZNodeFunction.h"
#include "JZNodeEvent.h"
#include "JZNodeView.h"
#include "JZNodeFlow.h"
#include "JZNodeValue.h"
#include "JZNodeFactory.h"
#include "JZNodeLocalParamEditDialog.h"
#include "JZNodeEditorManager.h"
#include "JZEditorGlobal.h"

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
    m_itemFunction = nullptr;
    m_itemClassDefine = nullptr;
    m_itemLocalDefine = nullptr;
    m_itemGlobalVariable = nullptr;
    m_itemLocalParam = nullptr;

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

QTreeWidgetItem *JZNodePanel::itemOp()
{
    return m_itemOp;
}

QTreeWidgetItem *JZNodePanel::itemProcess()
{
    return m_itemProcess;
}

void JZNodePanel::updateDefine()
{
    updateFunction();
    updateThis();
    updateLocalDefine();        
    updateGlobalVariable();
}

void JZNodePanel::updateThis()
{
    if (!m_classFile)
        return;

    auto def = m_classFile->objectDefine();
    QStringList func_list;
    for (int i = 0; i < def.functions.size(); i++)         
        func_list << def.functions[i].fullName();
        
    syncChildList(m_itemClassDefine->child(0), func_list, Create_Function);

    //params    
    QStringList params;
    params << "this" << def.paramList(false);
    for(int i = 1; i < params.size(); i++)
        params[i] = "this." + params[i];
    
    syncChildList(m_itemClassDefine->child(1), params, Create_Param);
}

void JZNodePanel::updateFunction()
{
    QStringList in_params, out_params, local_params;

    auto in_list = m_file->function().paramIn;    
    for (int i = 0; i < in_list.size(); i++)
        in_params << in_list[i].name;
    if (m_file->function().isMemberFunction())
        in_params.pop_front();

    auto out_list = m_file->function().paramOut;
    for (int i = 0; i < out_list.size(); i++)
        out_params << out_list[i].name;

    local_params = m_file->localVariableList(false);
    
    syncChildList(m_itemFunction->child(0), in_params, Create_Param);
    syncChildList(m_itemFunction->child(1), out_params, Create_Param);
    syncChildList(m_itemFunction->child(2), local_params, Create_Param);
}

void JZNodePanel::updateGlobalVariable()
{
    QStringList params = m_file->project()->globalVariableList();
    m_itemGlobalVariable->setHidden(params.size() == 0);
    syncChildList(m_itemGlobalVariable, params, Create_Param);
}

QStringList JZNodePanel::childItemText(QTreeWidgetItem *root)
{
    QStringList str_list;
    for (int i = 0; i < root->childCount(); i++)
    {
        str_list << root->child(i)->text(0);
    }
    return str_list;
}

void JZNodePanel::sortChildItem(QTreeWidgetItem *root)
{
    int count = root->childCount();
    QList<QTreeWidgetItem*> sort_list;
    for (int i = count - 1; i >= 0; i--)
    {
        sort_list << root->takeChild(i);
    }
    std::sort(sort_list.begin(), sort_list.end(), [](const QTreeWidgetItem *a,const QTreeWidgetItem *b)->bool {
        return a->text(0).toLower() < b->text(0).toLower();
    });

    for (int i = 0; i < sort_list.size(); i++)
        root->addChild(sort_list[i]);
}

void JZNodePanel::syncChildList(QTreeWidgetItem *root, QStringList new_list, int type)
{
    bool is_sort = false;

    //之前有序，删除也有序
    QList<QTreeWidgetItem*> need_remove;
    for (int i = 0; i < root->childCount(); i++)
    {
        QString name = root->child(i)->text(0);
        if (!new_list.contains(name))
            need_remove << root->child(i);
    }
    for (int i = 0; i < need_remove.size(); i++)
        root->removeChild(need_remove[i]);

    //新增，要重新排序
    QStringList cur_list = childItemText(root);
    for (int i = 0; i < new_list.size(); i++)
    {
        if (!cur_list.contains(new_list[i]))
        {
            if(type == Create_Function)
                root->addChild(createFunction(new_list[i]));
            else if (type == Create_Param)
                root->addChild(createParam(new_list[i]));
            else
                root->addChild(createClass(new_list[i]));

            is_sort = true;
        }
    }

    if(is_sort)
        sortChildItem(root);
}

void JZNodePanel::updateLocalDefine()
{
    QStringList function_list = m_file->project()->functionList();
    function_list.removeAll("main");

    QStringList class_list = m_file->project()->classList();        
    syncChildList(m_itemLocalDefine->child(0), function_list, Create_Function);
    syncChildList(m_itemLocalDefine->child(1), class_list, Create_Class);
}

void JZNodePanel::init()
{
    m_tree->clear();    

    m_itemFunction = createFolder("流程");
    m_tree->addTopLevelItem(m_itemFunction);
    m_itemFunction->addChild(createFolder("输入参数"));
    m_itemFunction->addChild(createFolder("输出参数"));

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
    m_itemFunction->addChild(m_itemLocalParam);
    m_tree->setItemWidget(m_itemLocalParam, 0, w);

    initBasic();    
    intiLogicFlow();

    m_itemClassDefine = createFolder("类");
    m_tree->addTopLevelItem(m_itemClassDefine);
    m_itemClassDefine->addChild(createFolder("成员函数"));
    m_itemClassDefine->addChild(createFolder("成员变量"));    
    if (!m_classFile) {
        m_itemClassDefine->setHidden(true);
    }       

    initLocalDefine();

    m_itemGlobalVariable = createFolder("全局变量");
    m_tree->addTopLevelItem(m_itemGlobalVariable);

    auto m_module = createFolder("所有");
    m_tree->addTopLevelItem(m_module);    
    initAll(m_module);

    updateDefine();        

    for (int i = 0; i < m_tree->topLevelItemCount() - 1; i++)
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
    item->setData(0,TreeItem_value, editorNodeFactory()->saveNode(node));
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

QTreeWidgetItem *JZNodePanel::createFunction(QString name)
{        
    auto coor = JZFunctionHelper::splitFunction(name);

    JZNodeFunction func_node;
    func_node.setFunction(name);

    QTreeWidgetItem *item = createNode(&func_node);
    item->setText(0, coor.name);
    return item;
}

QTreeWidgetItem * JZNodePanel::createClass(QString class_name)
{
    QTreeWidgetItem *item_class = new QTreeWidgetItem();
    item_class->setText(0, class_name);
    item_class->setData(0, TreeItem_isClass, true);
    item_class->setData(0, TreeItem_type, "node_class");
    item_class->setData(0, TreeItem_value, class_name);
    return item_class;
}

void JZNodePanel::initBasic()
{
    QTreeWidgetItem *item_basic = createFolder("基本");
    m_tree->addTopLevelItem(item_basic);

    QTreeWidgetItem *itemConst = createFolder("常量");
    initConstParam(itemConst);
    item_basic->addChild(itemConst);
    
    initProcess(item_basic);
    initExpression(item_basic);

    QTreeWidgetItem *itemOp = createFolder("操作");
    item_basic->addChild(itemOp);
    m_itemOp = itemOp;

    JZNodeParam node_param;
    itemOp->addChild(createNode(&node_param));

    JZNodeSetParam node_setParam;
    JZNodeCreateObject node_create;
    JZNodeCreateFromString node_createFormString;
    itemOp->addChild(createNode(&node_setParam));
    itemOp->addChild(createNode(&node_create));
    itemOp->addChild(createNode(&node_createFormString));
/*
    JZNodeMemberParam node_memberParam;
    JZNodeSetMemberParam node_setMemberParam;
    itemOp->addChild(createNode(&node_memberParam));
    itemOp->addChild(createNode(&node_setMemberParam));
    */
    JZNodeSwap node_swap;
    JZNodeClone node_clone;
    itemOp->addChild(createNode(&node_swap));
//  itemOp->addChild(createNode(&node_clone));

    JZNodeConvert node_convert;
    JZNodePrint node_print;
    JZNodeDisplay node_display;
    itemOp->addChild(createNode(&node_convert));
    itemOp->addChild(createNode(&node_print));
    itemOp->addChild(createNode(&node_display));
}

void JZNodePanel::intiLogicFlow()
{
    QTreeWidgetItem *item_logic = createFolder("业务流程");
    m_tree->addTopLevelItem(item_logic);

    auto logic_list = editorManager()->logicNodeList();
    for (int i = 0; i < logic_list.size(); i++)
    {
        auto &node = logic_list[i];
        
        QStringList path = node.path.split("/");
        QTreeWidgetItem *item = item_logic;
        for (int i = 0; i < path.size(); i++)
        {
            int sub_idx = UiHelper::treeIndexOf(item, path[i]);
            if (sub_idx >= 0)
                item = item->child(sub_idx);
            else
            {
                auto sub_item = createFolder(path[i]);
                item->addChild(sub_item);
                item = sub_item;
            }
        }
        auto jznode = editorNodeFactory()->createNode(node.nodeType);
        item->addChild(createNode(jznode));
        delete jznode;
    }
    sortChildItem(item_logic);
}

void JZNodePanel::initLocalDefine()
{
    QTreeWidgetItem *item_local = createFolder("本地");
    m_tree->addTopLevelItem(item_local);
    m_itemLocalDefine = item_local;        

    auto global_func = createFolder("函数");
    auto global_class = createFolder("类");
    m_itemLocalDefine->addChild(global_func);
    m_itemLocalDefine->addChild(global_class);    
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
    classList.sort(Qt::CaseInsensitive);
    for (int cls_idx = 0; cls_idx < classList.size(); cls_idx++)
    {   
        QTreeWidgetItem *item_class = createClass(classList[cls_idx]);
        item_module->addChild(item_class);
    }
}

void JZNodePanel::initThis(QTreeWidgetItem *root)
{       
    auto def = m_classFile->objectDefine();
    if(def.superName.isEmpty())
    {
        root->addChild(createClass(def.superName));
    }

    auto itemMemberParam = createFolder("成员变量");
    root->addChild(itemMemberParam);

    auto memberFunction = createFolder("成员函数");
    root->addChild(memberFunction);
    
    //QTreeWidgetItem *itemClassEvent = createFolder("事件");
    //root->addChild(itemClassEvent);
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

    auto item_bool = createNode(&node_bool);
    auto item_int = createNode(&node_int);
    auto item_int64 = createNode(&node_int64);
    auto item_double = createNode(&node_double);
    auto item_string = createNode(&node_string);
    auto item_null = createNode(&node_null);
    item_bool->setText(0, "bool");
    item_int->setText(0, "int");
    item_int64->setText(0, "int64");
    item_double->setText(0, "double");
    item_string->setText(0, "string");
    item_null->setText(0, "null");

    root->addChild(item_bool);
    root->addChild(item_int);
    root->addChild(item_int64);
    root->addChild(item_double);
    root->addChild(item_string);
    root->addChild(item_null);

    JZNodeFunctionPointer node_func;
    root->addChild(createNode(&node_func));
}


void JZNodePanel::initAll(QTreeWidgetItem *root)
{
    auto global_func = createFolder("全局函数");
    auto global_class = createFolder("类");
    root->addChild(global_func);    
    root->addChild(global_class);

    QStringList func_list = m_file->project()->functionList();
    func_list.sort(Qt::CaseInsensitive);

    QStringList class_list = m_file->project()->classList();
    class_list.sort(Qt::CaseInsensitive);

    auto func_inst = editorFunctionManager();
    auto list = func_inst->functionList();
    for (int i = 0; i < list.size(); i++)
    {
        QString func_name = list[i]->fullName();
        if(list[i]->className.isEmpty() && !func_list.contains(func_name))
            global_func->addChild(createFunction(func_name));
    }

    auto obj_list = editorObjectManager()->getClassList();
    for(int i = 0; i < obj_list.size(); i++)
    {
        QString class_name = obj_list[i];
        if (!class_list.contains(class_name))
            global_class->addChild(createClass(class_name));
    }    
}

void JZNodePanel::initExpression(QTreeWidgetItem *root)
{        
    QTreeWidgetItem *itemExpr = createFolder("运算符");

    for (int i = Node_add; i <= Node_expr; i++)
    {   
        auto node = editorNodeFactory()->createNode(i);
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
    JZNodeTryCatch node_try;
    JZNodeThrow node_throw;
    
    item_process->addChild(createNode(&node_nop));
    item_process->addChild(createNode(&node_sequence));
    item_process->addChild(createNode(&node_if));
    item_process->addChild(createNode(&node_switch));        
    item_process->addChild(createNode(&node_while));
    item_process->addChild(createNode(&node_for));
    item_process->addChild(createNode(&node_foreach));
    item_process->addChild(createNode(&node_continue));
    item_process->addChild(createNode(&node_break));        
    item_process->addChild(createNode(&node_return));
    item_process->addChild(createNode(&node_try));
    item_process->addChild(createNode(&node_throw));    

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
    return JZModuleManager::instance()->module(name);
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

QTreeWidgetItem *JZNodePanel::localVariableItem(QString name)
{
    int index = UiHelper::treeIndexOf(m_itemLocalParam, name);
    if (index == -1)
        return nullptr;

    return m_itemLocalParam->child(index);
}