#include "JZNodePanel.h"
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

//JZNodeCreateInfo
JZNodeCreateInfo JZNodeCreateInfo::fromBuffer(const QByteArray &buffer)
{
    QDataStream s(buffer);
    JZNodeCreateInfo info;
    s >> info.nodeType;
    s >> info.args;
    return info;
}

QByteArray JZNodeCreateInfo::toBuffer() const
{
    QByteArray buffer;
    QDataStream s(&buffer,QIODevice::WriteOnly);
    s << nodeType;
    s << args;
    return buffer;
}

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
    m_itemLocal = nullptr;
    m_itemInput = nullptr;

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
            m_functionMap[name] = functions;
        }
        file.close();
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

void JZNodePanel::updateNode()
{
    updateClass();    
    updateInput();
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

        QTreeWidgetItem *item = createNode(function_name,Node_function,{full_name});
        item->setText(0, def.functions[i].name);        
        m_memberFunction->addChild(item);
    }

    //params
    QStringList params;
    params << "this" << def.paramList(false);
    UiHelper::treeUpdate(m_memberParam, params, [this, &def, params](int index)->QTreeWidgetItem* {
        if (index == 0)
        {
            return createNode("this",Node_this);
        }
        else
        {
            auto info = &def.params[params[index]];
            QTreeWidgetItem *item = createParam("this." + params[index]);
            item->setText(0,params[index]);
            return item;
        }
    });
}

void JZNodePanel::updateInput()
{
    if (m_file->itemType() == ProjectItem_scriptFunction)
    {
        auto &function = m_file->function();
        int param_add = 0;
        QStringList params;
        for (int i = 0; i < function.paramIn.size(); i++)
        {
            if (i == 0 && function.paramIn[0].name == "this")
            {
                param_add = 1;
                continue;
            }
            params << function.paramIn[i].name;
        }

        UiHelper::treeUpdate(m_itemInput, params, [this,&function, param_add](int index)->QTreeWidgetItem*
        {
            auto info = &function.paramIn[index + param_add];
            QTreeWidgetItem *item = createParam(info->name);
            return item;
        });
        m_itemInput->setHidden(function.paramIn.size() == 0);
    }    
}

void JZNodePanel::init()
{
    m_tree->clear();

    initData();
    initBasicFlow();
    initFunction();
    initEnums();
    initClass();
           
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

QTreeWidgetItem *JZNodePanel::createNode(QString node_name,int node_type,const QStringList &args)
{
    JZNodeCreateInfo info;
    info.nodeType = node_type;
    info.args = args;

    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, node_name);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0, TreeItem_type, "node_data");
    item->setData(0,TreeItem_value,info.toBuffer());

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

QTreeWidgetItem * JZNodePanel::createClassEvent(QString className)
{
    auto meta = JZNodeObjectManager::instance()->meta(className);
    Q_ASSERT(meta);
    if (meta->singles.size() == 0)
        return nullptr;

    QTreeWidgetItem *item_class = new QTreeWidgetItem();
    item_class->setText(0, className);
           
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

    QTreeWidgetItem *itemOp = createFolder("操作");
    itemDataFolder->addChild(itemOp);    
 
    itemOp->addChild(createNode("get",Node_param));
    if (m_file->itemType() == ProjectItem_scriptParamBinding)
    {
        itemOp->addChild(createNode("set",Node_setParamData));
    }
    else
    {
        itemOp->addChild(createNode("set",Node_setParam));
        itemOp->addChild(createNode("createObject",Node_create));
        itemOp->addChild(createNode("createFromString",Node_createFromString));
    }   
    itemOp->addChild(createNode("getMember",Node_memberParam));
    itemOp->addChild(createNode("setMember",Node_setMemberParam));

    itemOp->addChild(createNode("swap",Node_swap));
    itemOp->addChild(createNode("clone",Node_clone));

    QTreeWidgetItem *itemParam = createFolder("全局变量");
    itemDataFolder->addChild(itemParam);
    initProjectParam(itemParam);
}

void JZNodePanel::initBasicFlow()
{
    QTreeWidgetItem *itemFlow = createFolder("操作");
    m_tree->addTopLevelItem(itemFlow);
    initProcess(itemFlow);

    QTreeWidgetItem *itemConvert = createFolder("类型转换");
    itemFlow->addChild(itemConvert);
    initConvert(itemConvert);

    QTreeWidgetItem *itemExpr = createFolder("运算符");
    itemFlow->addChild(itemExpr);
    initExpression(itemExpr);    
}

void JZNodePanel::initFunction()
{    
    auto addFunctions = [this](QTreeWidgetItem *item,const QStringList &functions, QSet<QString> &use)
    {
        for (int i = 0; i < functions.size(); i++)
        {
            auto *func = JZNodeFunctionManager::instance()->function(functions[i]);
            Q_ASSERT(func);

            auto function_node = createNode(func->name,Node_function,{func->fullName()});
            item->addChild(function_node);
            use.insert(functions[i]);
        }
    };     

    auto func_inst = JZNodeFunctionManager::instance();
    QSet<QString> function_used;

    QTreeWidgetItem *itemFunction = createFolder("函数");
    m_tree->addTopLevelItem(itemFunction);    

    QTreeWidgetItem *itemCustom = createFolder("自定义函数");
    itemFunction->addChild(itemCustom);

    auto user_functions = m_file->project()->functionList();
    addFunctions(itemCustom, user_functions, function_used);           

    //group
    auto it = m_functionMap.begin();
    while (it != m_functionMap.end())
    {
        QString name = it.key();
        QStringList functions = it.value();
        
        QTreeWidgetItem *item_group = createFolder(name);
        itemFunction->addChild(item_group);

        addFunctions(item_group, functions, function_used);
        it++;
    }    
}

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

void JZNodePanel::initClass()
{
    QTreeWidgetItem *item_class_root = createFolder("类");
    m_tree->addTopLevelItem(item_class_root);

    QMap<QString, QTreeWidgetItem*> itemMap;

    auto class_list = JZNodeObjectManager::instance()->getClassList();
    class_list.sort();
    for (int i = 0; i < class_list.size(); i++)
    {
        auto meta = JZNodeObjectManager::instance()->meta(class_list[i]);
        
        QString class_name = meta->className;
        
        QTreeWidgetItem *item_class;        
        item_class = new QTreeWidgetItem();
        item_class->setText(0, class_name);
        item_class->setData(0, TreeItem_isClass, true);
        itemMap[class_name] = item_class;
        item_class_root->addChild(item_class);
        
        for (int i = 0; i < meta->functions.size(); i++)
        {
            auto func = &meta->functions[i];
            JZNodeFunction node_func;
            node_func.setFunction(func);

            auto function_node = createNode(func->name,Node_function,{ func->fullName()});
            function_node->setText(0, func->name);
            item_class->addChild(function_node);
        }        
    }
}

void JZNodePanel::initThis(QTreeWidgetItem *root)
{       
    m_memberParam = createFolder("成员变量");
    root->addChild(m_memberParam);

    m_memberFunction = createFolder("成员函数");
    root->addChild(m_memberFunction);    
    
    auto def = m_classFile->objectDefine();
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
    
    if (m_file->itemType() == ProjectItem_scriptFunction)
    {
        m_itemInput = createFolder("输入参数");
        root->addChild(m_itemInput);
        updateInput();
    }

    m_itemLocal = createFolder("");
    root->addChild(m_itemLocal);

    auto list = m_file->localVariableList(false);
    m_tree->setItemWidget(m_itemLocal, 0, w);
    for (int i = 0; i < list.size(); i++)
    {
        auto info = m_file->localVariable(list[i]);
        QTreeWidgetItem *item = createParam(list[i]);        
        m_itemLocal->addChild(item);
    }
}

void JZNodePanel::initConstParam(QTreeWidgetItem *root)
{
    root->addChild(createNode("bool",Node_literal,{"bool"}));    
    root->addChild(createNode("int",Node_literal,{"int"}));
    root->addChild(createNode("int64",Node_literal,{"int64"}));
    root->addChild(createNode("double",Node_literal,{"double"}));
    root->addChild(createNode("null",Node_literal,{"null"}));

    
    root->addChild(createNode("display",Node_display));
    root->addChild(createNode("print", Node_print));
}

void JZNodePanel::initConvert(QTreeWidgetItem *root)
{    
}

void JZNodePanel::initExpression(QTreeWidgetItem *root)
{        
    for (int i = Node_add; i <= Node_expr; i++)
    {   
        auto node = JZNodeFactory::instance()->createNode(i);
        QTreeWidgetItem *sub = createNode(node->name(),i);
        root->addChild(sub);
        delete node;
    }
}

void JZNodePanel::initProcess(QTreeWidgetItem *root)
{
    QTreeWidgetItem *item_process = createFolder("过程");    ;

    item_process->addChild(createNode("branch",Node_branch));
    item_process->addChild(createNode("if",Node_if));
    item_process->addChild(createNode("switch",Node_switch));
    item_process->addChild(createNode("sequence",Node_sequence));
    
    item_process->addChild(createNode("while",Node_while));
    item_process->addChild(createNode("for",Node_for));
    item_process->addChild(createNode("foreach",Node_foreach));
    item_process->addChild(createNode("continue",Node_continue));
    item_process->addChild(createNode("break",Node_break));    

    item_process->addChild(createNode("nop",Node_nop));

    item_process->addChild(createNode("return",Node_return));
    item_process->addChild(createNode("exit",Node_exit));        
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
    if (item->parent() != m_itemLocal)
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
        m_itemLocal->removeChild(item);
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
    m_itemLocal->addChild(item);
    m_itemLocal->setExpanded(true);       
}