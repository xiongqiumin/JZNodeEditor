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
#include "JZNodeFunctionManager.h"
#include "JZNodeValue.h"
#include "JZNodeFactory.h"
#include "JZProjectItem.h"
#include "JZNodeObject.h"
#include "JZProject.h"
#include "JZScriptFile.h"
#include "JZNodeFunction.h"
#include "JZNodeEvent.h"
#include "JZNodeParamEditDialog.h"

enum{
    TreeItem_type = Qt::UserRole,
    TreeItem_value,
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

void JZNodePanel::setFile(JZScriptFile *file)
{
    m_file = file;    
    m_classFile = m_file->project()->getClassFile(m_file);
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
        JZNodeFunction node;
        node.setFunction(&def.functions[i]);

        QTreeWidgetItem *item = createNode(&node);
        item->setText(0, def.functions[i].name);        
        m_memberFunction->addChild(item);
    }

    //params
    QStringList params;
    params << "this" << def.paramList(false);
    UiHelper::treeUpdate(m_memberParam, params, [this, &def, params](int index)->QTreeWidgetItem* {
        if (index == 0)
        {
            JZNodeThis node_this;
            return createNode(&node_this);
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
    initEvent();
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

QTreeWidgetItem *JZNodePanel::createNode(JZNode *node)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, node->name());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0, TreeItem_type, "node_data");
    item->setData(0, TreeItem_value, saveNode(node));
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
           
    for (int i = 0; i < meta->singles.size(); i++)
    {        
        JZNodeSingleEvent node_event;
        node_event.setName(meta->singles[i].name);
        node_event.setSingle(className, &meta->singles[i]);

        QTreeWidgetItem *sub_event = new QTreeWidgetItem();
        sub_event->setData(0, TreeItem_type, "node_data");
        sub_event->setText(0, meta->singles[i].name);
        sub_event->setData(0, TreeItem_value, saveNode(&node_event));
        item_class->addChild(sub_event);                   
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

    QTreeWidgetItem *itemOp = createFolder("操作");
    itemDataFolder->addChild(itemOp);    

    JZNodeParam get;
    JZNodeSetParam set;
    JZNodeSetParamDataFlow set_data;

    JZNodeMemberParam getMember;
    JZNodeSetMemberParam setMember;    

    JZNodeCreate create;
    JZNodeClone swap;
    JZNodeSwap clone;    
    itemOp->addChild(createNode(&get));
    if (m_file->itemType() == ProjectItem_scriptParamBinding)
    {
        itemOp->addChild(createNode(&set_data));
    }
    else
    {
        itemOp->addChild(createNode(&set));
        itemOp->addChild(createNode(&create));
    }   
    itemOp->addChild(createNode(&getMember));
    itemOp->addChild(createNode(&setMember));

    itemOp->addChild(createNode(&swap));
    itemOp->addChild(createNode(&clone));

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

void JZNodePanel::initEvent()
{
    QTreeWidgetItem *item_event = createFolder("事件");
    m_tree->addTopLevelItem(item_event);
/*
    JZNodeParamChangedEvent event;
    item_event->addChild(createNode(&event));
*/
    QTreeWidgetItem *item_widget_event = createFolder("控件事件");
    item_event->addChild(item_widget_event);

    QTreeWidgetItem *item_object_event = createFolder("其他事件");
    item_event->addChild(item_object_event);

    auto inst = JZNodeObjectManager::instance();
    auto list = inst->getClassList();
    for (int i = 0; i < list.size(); i++)
    {        
        QTreeWidgetItem *item_class = createClassEvent(list[i]);
        if (item_class)
        {
            if (inst->isInherits(list[i], "Widget"))
                item_widget_event->addChild(item_class);
            else
                item_object_event->addChild(item_class);
        }
    }
}

void JZNodePanel::initFunction()
{    
    auto addFunctions = [this](QTreeWidgetItem *item,const QStringList &functions, QSet<QString> &use)
    {
        for (int i = 0; i < functions.size(); i++)
        {
            auto *func = JZNodeFunctionManager::instance()->function(functions[i]);
            Q_ASSERT(func);

            JZNodeFunction node_func;
            node_func.setName(functions[i]);
            node_func.setFunction(func);

            auto function_node = createNode(&node_func);
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

    QTreeWidgetItem *itemOther = createFolder("其他");
    itemFunction->addChild(itemOther);

    auto funcs = JZNodeFunctionManager::instance()->functionList();
    for (int i = 0; i < funcs.size(); i++)
    {
        QString function = funcs[i]->name;
        if (function.indexOf(".") != -1 || function_used.contains(function))
            continue;

        JZNodeFunction node_func;
        node_func.setName(function);
        node_func.setFunction(funcs[i]);

        auto function_node = createNode(&node_func);
        function_node->setText(0, function);
        itemOther->addChild(function_node);
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
        auto meta = JZNodeObjectManager::instance()->enumMeta(enum_list[i]);

        QString enum_name = meta->name();

        JZNodeEnum node_enum;
        node_enum.setEnmu(meta->type());

        QTreeWidgetItem *item_enum = createNode(&node_enum);        
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
        itemMap[class_name] = item_class;
        item_class_root->addChild(item_class);
        
        for (int i = 0; i < meta->functions.size(); i++)
        {
            auto func = &meta->functions[i];
            if (meta->event(func->name))
                continue;
            
            JZNodeFunction node_func;
            node_func.setFunction(func);

            auto function_node = createNode(&node_func);
            function_node->setText(0, func->name);
            item_class->addChild(function_node);
        }        
    }
}

void JZNodePanel::initThis(QTreeWidgetItem *root)
{   
    auto def = m_classFile->objectDefine();
    QTreeWidgetItem *itemClassEvent = createFolder("事件");
    root->addChild(itemClassEvent);

    auto event_list = def.eventList();
    for (int i = 0; i < def.eventList().size(); i++)
    {
        auto event = def.event(event_list[i]);

        JZNodeQtEvent node;
        node.setEvent(def.className, event);
        QTreeWidgetItem *item = createNode(&node);
        itemClassEvent->addChild(item);
    }

    m_memberFunction = createFolder("成员函数");
    root->addChild(m_memberFunction);    

    //成员变量
    m_memberParam = createFolder("成员变量");
    root->addChild(m_memberParam);    

    updateClass();
}

void JZNodePanel::initProjectParam(QTreeWidgetItem *root)
{
    auto project = m_file->project();
    auto list = project->globalVariableList();
    for(int i = 0; i < list.size(); i++)
    {
        auto info = project->globalVariableInfo(list[i]);
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
        auto info = m_file->localVariableInfo(list[i]);
        QTreeWidgetItem *item = createParam(list[i]);        
        m_itemLocal->addChild(item);
    }
}

void JZNodePanel::initConstParam(QTreeWidgetItem *root)
{
    JZNodeLiteral node_int,node_bool,node_string,node_double,node_null;
    node_int.setName("整数");
    node_int.setLiteral(0);
    node_int.setDataType(Type_int);

    node_bool.setName("Bool");
    node_bool.setLiteral(false);
    node_bool.setDataType(Type_bool);

    node_string.setName("字符串");
    node_string.setLiteral("");
    node_string.setDataType(Type_string);

    node_double.setName("浮点数");
    node_double.setLiteral(0.0);
    node_double.setDataType(Type_double);

    node_null.setName("null");    
    node_null.setLiteral("null");
    node_null.setDataType(Type_nullptr);

    root->addChild(createNode(&node_int));    
    root->addChild(createNode(&node_string));
    root->addChild(createNode(&node_double));
    root->addChild(createNode(&node_bool));
    root->addChild(createNode(&node_null));
}

void JZNodePanel::initConvert(QTreeWidgetItem *root)
{
    QStringList funcList = { "toInt","toString","toDouble","toBool" };

    for (int i = 0; i < funcList.size(); i++)
    {
        QString function = funcList[i];
        auto func_ptr = JZNodeFunctionManager::instance()->function(function);
        
        JZNodeFunction node_func;
        node_func.setName(function);
        node_func.setFunction(func_ptr);

        auto function_node = createNode(&node_func);
        function_node->setText(0, function);
        root->addChild(function_node);
    }
    
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
    QTreeWidgetItem *item_process = createFolder("过程");    

    JZNodeFor node_for;
    JZNodeWhile node_while;
    JZNodeSequence node_seq;
    JZNodeBranch node_branch;
    JZNodeIf node_if;
    JZNodeSwitch node_switch;
    JZNodeForEach node_foreach;
    JZNodeBreak node_break;
    JZNodeContinue node_continue;
    JZNodeExit node_exit;
    JZNodeReturn node_return;
    JZNodeNop node_nop;
    if (m_file->itemType() == ProjectItem_scriptFunction) 
    {
        node_return.setFunction(&m_file->function());
    }
    
    item_process->addChild(createNode(&node_branch));
    item_process->addChild(createNode(&node_if));
    item_process->addChild(createNode(&node_switch));
    item_process->addChild(createNode(&node_seq));
    
    item_process->addChild(createNode(&node_while));
    item_process->addChild(createNode(&node_for));
    item_process->addChild(createNode(&node_foreach));
    item_process->addChild(createNode(&node_continue));
    item_process->addChild(createNode(&node_break));    

    item_process->addChild(createNode(&node_nop));

    item_process->addChild(createNode(&node_return));
    item_process->addChild(createNode(&node_exit));        
    root->addChild(item_process);
}

bool JZNodePanel::filterItem(QTreeWidgetItem *item,QString name)
{
    bool show = false;
    int count = item->childCount();
    if(count == 0)
    {
        show = item->text(0).contains(name);
    }
    else
    {
        for(int i = 0; i < count; i++)
        {
            if(filterItem(item->child(i),name))
                show = true;
        }
    }
    item->setHidden(!show);
    return show;
}

void JZNodePanel::onSearch()
{
    QString name = m_lineSearch->text();
    filterItem(m_tree->invisibleRootItem(),name);
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
        auto def = m_file->localVariableInfo(item->text(0));

        JZNodeParamEditDialog dialog(this);;
        dialog.init(m_file, *def);
        if (dialog.exec() != QDialog::Accepted)
            return;

        auto new_def = dialog.param();
        if(new_def.name != def->name || new_def.dataType != def->dataType
            || new_def.value != def->value)
            m_file->replaceLocalVariableInfo(def->name,new_def);
    }
    else if (act == actDel)
    {
        m_file->removeLocalVariable(item->text(0));
        m_itemLocal->removeChild(item);
    }
}

void JZNodePanel::onAddScriptParam()
{
    QString name;
    auto list = m_file->localVariableList(true);
    int i = 0;
    while(true)
    {
        name = "local" + QString::number(i+1);
        if (!list.contains(name))
            break;
        i++;
    }
    
    JZParamDefine def;
    def.name = name;
    def.dataType = Type_int;

    JZNodeParamEditDialog dialog(this);;
    dialog.init(m_file, def);
    if (dialog.exec() != QDialog::Accepted)
        return;
    
    m_file->addLocalVariable(dialog.param());
    QTreeWidgetItem *item = createParam(name);
    m_itemLocal->addChild(item);
    m_itemLocal->setExpanded(true);       
}