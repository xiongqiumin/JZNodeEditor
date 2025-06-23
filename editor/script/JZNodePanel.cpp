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

// JZNodePanel
JZNodePanel::JZNodePanel(QWidget *widget)
    : JZNodeAbstractPanel(widget)
{    
    m_classFile = nullptr;
    m_itemFunction = nullptr;
    m_itemClassDefine = nullptr;
    m_itemLocalDefine = nullptr;
    m_itemGlobalVariable = nullptr;    
}

JZNodePanel::~JZNodePanel()
{
}

void JZNodePanel::init()
{
    m_tree->clear();

    m_itemFunction = createFolder("流程");
    m_tree->addTopLevelItem(m_itemFunction);
    m_itemFunction->addChild(createFolder("输入参数"));
    m_itemFunction->addChild(createFolder("输出参数"));    
    
    initLocalParam(m_itemFunction);

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

void JZNodePanel::updateDefine()
{
    updateFunction();
    updateThis();    
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
    for (int i = 1; i < params.size(); i++)
        params[i] = "this." + params[i];

    syncChildList(m_itemClassDefine->child(1), params, Create_Param);
}

void JZNodePanel::updateFunction()
{
    QStringList in_params, out_params;

    auto in_list = m_file->function().paramIn;
    for (int i = 0; i < in_list.size(); i++)
        in_params << in_list[i].name;
    if (m_file->function().isMemberFunction())
        in_params.pop_front();

    auto out_list = m_file->function().paramOut;
    for (int i = 0; i < out_list.size(); i++)
        out_params << out_list[i].name;
    
    syncChildList(m_itemFunction->child(0), in_params, Create_Param);
    syncChildList(m_itemFunction->child(1), out_params, Create_Param);
    updateLocalParam();    
}

void JZNodePanel::updateGlobalVariable()
{
    QStringList params = m_file->project()->globalVariableList();
    m_itemGlobalVariable->setHidden(params.size() == 0);
    syncChildList(m_itemGlobalVariable, params, Create_Param);
}

void JZNodePanel::updateLocalDefine()
{
    QStringList function_list = m_file->project()->functionList();
    function_list.removeAll("main");

    QStringList class_list = m_file->project()->classList();
    syncChildList(m_itemLocalDefine->child(0), function_list, Create_Function);
    syncChildList(m_itemLocalDefine->child(1), class_list, Create_Class);
}

void JZNodePanel::initBasic()
{
    QTreeWidgetItem *item_basic = createFolder("基本");
    m_tree->addTopLevelItem(item_basic);

    QTreeWidgetItem *itemConst = createFolder("常量");
    initConstParam(itemConst);
    item_basic->addChild(itemConst);

    initProcess(item_basic);
    initExpression(item_basic,false);

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
        auto sub_item = createNode(jznode);
        if (!node.icon.isEmpty())
            sub_item->setIcon(0, QIcon(node.icon));

        item->addChild(sub_item);
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

void JZNodePanel::addModule(QTreeWidgetItem *item_root, QString name)
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
        Q_ASSERT_X(func, "Error Function", qUtf8Printable(func_name));

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
    if (def.superName.isEmpty())
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
    JZNodeLiteral node_byteArray;

    node_bool.setDataType(Type_bool);
    node_int.setDataType(Type_int);
    node_int64.setDataType(Type_int64);
    node_double.setDataType(Type_double);
    node_string.setDataType(Type_string);
    node_byteArray.setDataType(Type_byteArray);
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

    QStringList local_func_list = m_file->project()->functionList();
    QStringList local_class_list = m_file->project()->classList();

    auto func_inst = editorFunctionManager();
    auto list = func_inst->functionList();
    std::sort(list.begin(), list.end(), name_sort);
    for (int i = 0; i < list.size(); i++)
    {
        QString func_name = list[i];
        if (!local_func_list.contains(func_name))
            global_func->addChild(createFunction(func_name));
    }

    bool flag = QString("QBrush") < QString("QImage");
    flag = name_sort(QString("QBrush"), QString("QImage"));
    auto obj_list = editorObjectManager()->getClassList();
    std::sort(obj_list.begin(), obj_list.end(), name_sort);
    for (int i = 0; i < obj_list.size(); i++)
    {
        QString class_name = obj_list[i];
        if (!local_class_list.contains(class_name))
            global_class->addChild(createClass(class_name));
    }
}