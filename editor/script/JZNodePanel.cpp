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
#include "UiCommon.h"

enum{
    TreeItem_type = Qt::UserRole,
    TreeItem_value,
    TreeItem_isClass,
};

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
    m_classFile = nullptr;    
    m_memberFunction = nullptr;
    m_memberParam = nullptr;
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

    QFile file(":/JZNodeEditor/Resources/fucntionList.txt");
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream s(&file);
        s.setCodec("utf-8");
        while (!s.atEnd())
        {
            QString line = s.readLine();
            int idx = line.indexOf(":");
            if (idx == -1)
                continue;

            QString name = line.mid(0, idx);
            QStringList functions = line.mid(idx + 1).split(",");
            Module m;
            m.name = name;
            m.functionList = functions;
            m_modules.push_back(m);
        }
        file.close();
    }

    Module core;
    core.name = "core";
    core.typeList << "QObject" << "QTimer" << "QPoint" << "QPointF" << "QRect" << "QRectF" << "QColor" << "QFile";
    core.typeList << "QList<int>" << "QList<double>" << "QStringList" << "QMap<int,int>" << "QMap<int,QString>" << "QMap<QString,QString>" << "QMap<QString,int>";  
    m_modules.push_back(core);

    Module gui;
    gui.name = "gui";
    gui.typeList << "QWidget" << "QLabel" << "QLineEdit" << "QTextEdit" << "QPushButton" << "QRadioButton" << "QToolButton" << "QCheckBox" << "QComboBox"
        << "QPainter" << "QPen" << "QBrush" << "QKeyEvent" << "QMouseEvent" << "QShowEvent" << "QPaintEvent" << "QResizeEvent" << "Qt::Key"; 
    m_modules.push_back(gui);
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

void JZNodePanel::updateNode()
{
    updateClass();    
    updateLocalVariable();
}

void JZNodePanel::updateClass()
{
    if (!m_classFile)
        return;

    auto def = m_classFile->objectDefine();
    //function
    UiHelper::clearTreeItem(m_memberFunction);    
    for (int i = 0; i < def.functions.size(); i++)
    {
        QString function_name = def.functions[i].name;
        QString full_name = def.functions[i].fullName();

        JZNodeFunction node_function;
        node_function.setFunction(full_name);
        QTreeWidgetItem *item = createNode(&node_function);
        item->setText(0, def.functions[i].name);        
        m_memberFunction->addChild(item);
    }

    //params
    UiHelper::clearTreeItem(m_memberParam);
    QStringList params;
    params << "this" << def.paramList(false);
    for(int i = 0; i < params.size(); i++)
    {
        QTreeWidgetItem *item = nullptr;
        if (i == 0)
        {   
            JZNodeThis node_this;
            item = createNode(&node_this);
        }
        else
        {
            item = createParam("this." + params[i]);
            item->setText(0,params[i]);
        }
        m_memberParam->addChild(item);
    }
}

void JZNodePanel::updateLocalVariable()
{
    if (m_file->itemType() == ProjectItem_scriptFunction)
    {
        QStringList params = m_file->localVariableList(true);
        UiHelper::clearTreeItem(m_itemLocalParam);
        for(int i = 0; i < params.size(); i++)
        {
            QTreeWidgetItem *item = createParam(params[i]);
            m_itemLocalParam->addChild(item);
        }
    }    
}

void JZNodePanel::init()
{
    m_tree->clear();

    initData();
    initBasicFlow();
    initLocalDefine();
    initModule();
           
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

QTreeWidgetItem *JZNodePanel::createNode(JZNode *node)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, node->name());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0, TreeItem_type, "node_data");
    item->setData(0,TreeItem_value, JZNodeFactory::instance()->saveNode(node));
    return item;
}

QTreeWidgetItem *JZNodePanel::createParam(QString name)
{
    QString full_name = name;
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0,TreeItem_type,"node_param");    
    item->setData(0,TreeItem_value,full_name);

    return item;
}

QTreeWidgetItem * JZNodePanel::createClass(QString class_name)
{
    auto enum_meta = JZNodeObjectManager::instance()->enumMeta(class_name);
    if(enum_meta)
    {
        JZNodeEnum node_enum;
        node_enum.setEnum(class_name);
        QTreeWidgetItem *item_enum = createNode(&node_enum);
        return item_enum;
    }

    auto meta = JZNodeObjectManager::instance()->meta(class_name);
    Q_ASSERT_X(meta,"Error Class",qUtf8Printable(class_name));

    QTreeWidgetItem *item_class = new QTreeWidgetItem();
    item_class->setText(0, class_name);
    item_class->setData(0, TreeItem_isClass, true);

    if(!meta->superName.isEmpty())
    {
        auto item_super = createClass(meta->superName);
        item_class->addChild(item_super);
    }

    for (int i = 0; i < meta->enums.size(); i++)
    {        
        JZNodeEnum node_enum;
        node_enum.setEnum(meta->enums[i]);
        QTreeWidgetItem *item_enum = createNode(&node_enum);        
        item_class->addChild(item_enum);        
    }
    
    for (int func_idx = 0; func_idx < meta->functions.size(); func_idx++)
    {
        auto func = &meta->functions[func_idx];
        if(func->isProtected)
            continue;

        JZNodeFunction node_func;
        node_func.setFunction(func);

        auto function_node = createNode(&node_func);
        function_node->setText(0, func->name);
        item_class->addChild(function_node);
    }
           
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
    
    initScriptParam(itemDataFolder);

    QTreeWidgetItem *itemParam = createFolder("全局变量");
    itemDataFolder->addChild(itemParam);
    initProjectParam(itemParam);

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

void JZNodePanel::initBasicFlow()
{
    QTreeWidgetItem *itemFlow = createFolder("操作");
    m_tree->addTopLevelItem(itemFlow);
    initProcess(itemFlow);

    QTreeWidgetItem *itemExpr = createFolder("运算符");
    itemFlow->addChild(itemExpr);
    initExpression(itemExpr);    
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

    QStringList class_list = m_file->project()->classList();
    class_list << m_file->project()->containerList();
    for(int i = 0; i < class_list.size(); i++)
    {
        item_local->addChild(createClass(class_list[i]));
    }

    QStringList function_list = m_file->project()->functionList();
    auto item_local_func = createFolder("函数");
    for(int i = 0; i < function_list.size(); i++)
    {
        JZNodeFunction node_function;
        node_function.setFunction(function_list[i]);
        QTreeWidgetItem *func_item = createNode(&node_function);
        item_local_func->addChild(func_item);
    }
}

void JZNodePanel::initModule()
{
    auto func_inst = JZNodeFunctionManager::instance();

    QTreeWidgetItem *item_class_root = createFolder("模块");
    m_tree->addTopLevelItem(item_class_root);

    //class 
    for(int md_idx = 0; md_idx < m_modules.size(); md_idx++)
    {
        auto &m = m_modules[md_idx];

        auto item_module = createFolder(m.name);
        item_class_root->addChild(item_module);

        for (int func_idx = 0; func_idx < m.functionList.size(); func_idx++)
        {
            QString func_name = m.functionList[func_idx];
            auto *func = JZNodeFunctionManager::instance()->function(func_name);
            Q_ASSERT_X(func,"Error Function",qUtf8Printable(func_name));

            JZNodeFunction node_func;
            node_func.setFunction(func);
        
            auto function_node = createNode(&node_func);
            item_module->addChild(function_node);
        }

        for (int cls_idx = 0; cls_idx < m.typeList.size(); cls_idx++)
        {   
            QTreeWidgetItem *item_class = createClass(m.typeList[cls_idx]);        
            item_module->addChild(item_class);
        }
    }
}

void JZNodePanel::initThis(QTreeWidgetItem *root)
{       
    auto def = m_classFile->objectDefine();
    if(def.superName.isEmpty())
    {
        root->addChild(createClass(def.superName));
    }

    m_memberParam = createFolder("成员变量");
    root->addChild(m_memberParam);

    m_memberFunction = createFolder("成员函数");
    root->addChild(m_memberFunction);    
    
    QTreeWidgetItem *itemClassEvent = createFolder("事件");
    root->addChild(itemClassEvent);

    updateClass();
}

void JZNodePanel::initProjectParam(QTreeWidgetItem *root)
{
    auto project = m_file->project();
    auto list = project->globalVariableList();
    for(int i = 0; i < list.size(); i++)
    {
        auto info = project->globalVariable(list[i]);
        QTreeWidgetItem *item = createParam(list[i]);
        root->addChild(item);
    }
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
    updateLocalVariable();
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
    for (int i = Node_add; i <= Node_expr; i++)
    {   
        auto node = JZNodeFactory::instance()->createNode(i);
        QTreeWidgetItem *sub = createNode(node);
        root->addChild(sub);
        delete node;
    }
}

void JZNodePanel::initProcess(QTreeWidgetItem *root)
{
    QTreeWidgetItem *item_process = createFolder("过程");    ;
    
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
        m_file->setLocalVariable(old_def->name, new_def);
        item->setText(0,new_def.name);
    }
    else if (act == actDel)
    {
        m_file->removeLocalVariable(item->text(0));
        m_itemLocalParam->removeChild(item);
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
    m_file->addLocalVariable(define);
    QTreeWidgetItem *item = createParam(define.name);
    m_itemLocalParam->addChild(item);
    m_itemLocalParam->setExpanded(true);       
}