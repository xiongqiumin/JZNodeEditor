#include "JZNodePanel.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include "JZNodeFunctionManager.h"
#include "JZNodeValue.h"
#include "JZNodeFactory.h"
#include "JZProjectItem.h"
#include "JZNodeObject.h"
#include "JZProject.h"
#include "JZScriptFile.h"

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
    m_propEditor = nullptr;

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

    layout->addWidget(m_tree);
    setLayout(layout);
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

void JZNodePanel::setPropertyEditor(JZNodePropertyEditor *propEditor)
{
    m_propEditor = propEditor;
    connect(m_propEditor, &JZNodePropertyEditor::sigPropChanged, this, &JZNodePanel::onPropUpdate);
}

void JZNodePanel::init()
{
    m_tree->clear();

    initData();
    initBasicFlow();
    initEvent();
    initFunction();
    initClass();
           
    m_tree->expandAll();
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
    item->setData(0, TreeItem_value, formatNode(node));
    return item;
}

QTreeWidgetItem *JZNodePanel::createParam(QString name,int dataType,QString preName)
{
    QString full_name = name;
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0,TreeItem_type,"node_param");
    if(!preName.isEmpty())
        full_name = preName + "." + name;
    item->setData(0,TreeItem_value,full_name);

    if(JZNodeType::isObject(dataType))
    {
        auto def = JZNodeObjectManager::instance()->meta(dataType);
        auto list = def->paramList();
        for(int i = 0; i < list.size(); i++)
        {
            auto sub_def = def->param(list[i]);
            QTreeWidgetItem *sub_item = createParam(list[i],sub_def->dataType,full_name);
            item->addChild(sub_item);
        }
    }

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
        sub_event->setData(0, TreeItem_value, formatNode(&node_event));
        item_class->addChild(sub_event);                   
    }
    return item_class;
}

void JZNodePanel::initData()
{
    QTreeWidgetItem *itemDataFolder = createFolder("数据");
    m_tree->addTopLevelItem(itemDataFolder);

    QTreeWidgetItem *itemConst = createFolder("常量");
    itemDataFolder->addChild(itemConst);
    initConstParam(itemConst);

    QTreeWidgetItem *itemParam = createFolder("全局变量");
    itemDataFolder->addChild(itemParam);
    initProjectParam(itemParam);

    if (m_classFile) {
        QTreeWidgetItem *itemClassParam = createFolder("成员变量");        
        itemDataFolder->addChild(itemClassParam);

        JZNodeThis node_this;
        itemClassParam->addChild(createNode(&node_this));

        initClassParam(itemClassParam);
    }

    QTreeWidgetItem *itemLocalParam = createFolder("局部变量");
    itemDataFolder->addChild(itemLocalParam);
    initScriptParam(itemLocalParam);    

    QTreeWidgetItem *itemOp = createFolder("操作");
    itemDataFolder->addChild(itemOp);

    JZNodeParam get;
    JZNodeSetParam set;
    JZNodeSetParamDataFlow set_data;
    JZNodeCreate create;
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

    JZNodeParamChangedEvent event;
    item_event->addChild(createNode(&event));

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
            if (inst->isInherits(list[i], "widget"))
                item_widget_event->addChild(item_class);
            else
                item_object_event->addChild(item_class);
        }
    }
}

void JZNodePanel::initFunction()
{
    QTreeWidgetItem *itemFunction = createFolder("函数");
    m_tree->addTopLevelItem(itemFunction);

    QTreeWidgetItem *itemCustom = createFolder("自定义函数");
    itemFunction->addChild(itemCustom);

    QTreeWidgetItem *itemOther = createFolder("其他");
    itemFunction->addChild(itemOther);

    auto funcs = JZNodeFunctionManager::instance()->functionList();
    for (int i = 0; i < funcs.size(); i++)
    {
        QString function = funcs[i]->name;
        if (function.indexOf(".") != -1)
            continue;

        JZNodeFunction node_func;
        node_func.setName(function);
        node_func.setFunction(funcs[i]);

        auto function_node = createNode(&node_func);
        function_node->setText(0, function);
        itemFunction->addChild(function_node);
    }
}

void JZNodePanel::initClass()
{
    QTreeWidgetItem *item_class_root = createFolder("类");
    m_tree->addTopLevelItem(item_class_root);

    QMap<QString, QTreeWidgetItem*> itemMap;

    auto funcs = JZNodeFunctionManager::instance()->functionList();
    for (int i = 0; i < funcs.size(); i++)
    {
        QString function = funcs[i]->name;
        int index = function.indexOf(".");
        if (index == -1)
            continue;
        
        QString class_name, short_name;
        class_name = function.left(index);
        short_name = function.mid(index + 1);
        
        QTreeWidgetItem *item_class;
        if (!itemMap.contains(class_name))
        {
            item_class = new QTreeWidgetItem();
            item_class->setText(0, class_name);
            itemMap[class_name] = item_class;
            item_class_root->addChild(item_class);
        }
        else
        {
            item_class = itemMap[class_name];
        }

        JZNodeFunction node_func;
        node_func.setName(function);
        node_func.setFunction(funcs[i]);

        auto function_node = createNode(&node_func);
        function_node->setText(0, short_name);
        item_class->addChild(function_node);
    }
}

void JZNodePanel::initClassParam(QTreeWidgetItem *root)
{    
    auto def = m_classFile->objectDefine();
    auto list = def.paramList();
    for(int i = 0; i < list.size(); i++)
    {
        auto info = &def.params[list[i]];
        QTreeWidgetItem *item = createParam(list[i],info->dataType,"this");
        root->addChild(item);
    }
}

void JZNodePanel::initProjectParam(QTreeWidgetItem *root)
{
    auto project = m_file->project();
    auto list = project->globalVariableList();
    for(int i = 0; i < list.size(); i++)
    {
        auto info = project->globalVariableInfo(list[i]);
        QTreeWidgetItem *item = createParam(list[i],info->dataType,"");
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
    l->addWidget(new QLabel(root->text(0)));
    l->addStretch();
    l->addWidget(btn);
    w->setLayout(l);
    root->setText(0, "");
    m_itemLocalVariable = root;

    auto list = m_file->localVariableList();
    m_tree->setItemWidget(root, 0, w);

    for (int i = 0; i < list.size(); i++)
    {
        auto info = m_file->localVariableInfo(list[i]);
        QTreeWidgetItem *item = createParam(list[i], info->dataType, "");        
        root->addChild(item);
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
    QTreeWidgetItem *item_process = new QTreeWidgetItem();
    item_process->setText(0,"过程");    

    JZNodeFor node_for;
    JZNodeWhile node_while;
    JZNodeSequence node_seq;
    JZNodeBranch node_branch;
    JZNodeForEach node_foreach;
    JZNodeBreak node_break;
    JZNodeContinue node_continue;
    JZNodeExit node_exit;
    JZNodeReturn node_return;
    if (m_file->itemType() == ProjectItem_scriptFunction) 
    {
        node_return.setFunction(m_file->function());
    }

    item_process->addChild(createNode(&node_seq));
    item_process->addChild(createNode(&node_branch));
    item_process->addChild(createNode(&node_while));
    item_process->addChild(createNode(&node_for));
    item_process->addChild(createNode(&node_foreach));
    item_process->addChild(createNode(&node_break));
    item_process->addChild(createNode(&node_continue));
    item_process->addChild(createNode(&node_exit));
    item_process->addChild(createNode(&node_return));

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
    if (item->parent() != m_itemLocalVariable)
    {
        m_propEditor->setParamDefine(nullptr);
        return;
    }

    auto info = m_file->localVariableInfo(item->text(0));
    m_propEditor->setParamDefine(info);    
}

void JZNodePanel::onPropUpdate(int prop_id, const QVariant &value)
{
    auto item = m_tree->currentItem();
    Q_ASSERT(item);

    if (prop_id == PropEditor_varName)
    {
        QString oldName = item->text(0);
        item->setText(0, value.toString());

        item->setData(0, TreeItem_value, value.toString());
        m_file->renameLocalVariable(oldName, value.toString());
    }
    else if (prop_id == PropEditor_varType)
    {
        m_file->setLocalVariableType(item->text(0), value.toInt());
    }
}

void JZNodePanel::onAddScriptParam()
{
    QString name;
    auto list = m_file->localVariableList();
    int i = 0;
    while(true)
    {
        name = "新建变量" + QString::number(i+1);
        if (!list.contains(name))
            break;
        i++;
    }
    m_file->addLocalVariable(name, Type_int);

    QTreeWidgetItem *item = createParam(name, Type_int);
    m_itemLocalVariable->addChild(item);
}