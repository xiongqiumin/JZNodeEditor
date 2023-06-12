#include "JZNodePanel.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include "JZNodeFunctionManager.h"
#include "JZNodeValue.h"
#include "JZNodeFactory.h"
#include "JZProjectItem.h"

// JZNodeTreeWidget
QMimeData *JZNodeTreeWidget::mimeData(const QList<QTreeWidgetItem *> items) const
{
    Q_ASSERT(items.size() == 1);
    auto item = items[0];

    QMimeData *mimeData = new QMimeData();
    mimeData->setData("node_data", item->data(0, Qt::UserRole).toByteArray());
    return mimeData;
}

// JZNodePanel
JZNodePanel::JZNodePanel(QWidget *widget)
    : QWidget(widget)
{
    m_fileType = 0;

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);

    m_tree = new JZNodeTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);
    m_tree->setDragEnabled(true);   

    layout->addWidget(m_tree);
    setLayout(layout);
}

JZNodePanel::~JZNodePanel()
{
}

void JZNodePanel::init(int fileType)
{
    m_fileType = fileType;
    m_tree->clear();

    if(m_fileType != ProjectItem_scriptParamBinding)
    {
        QTreeWidgetItem *itemEvent = createFolder("事件");
        m_tree->addTopLevelItem(itemEvent);
        initEvent(itemEvent);
    }

    QTreeWidgetItem *itemData = createFolder("数据");
    m_tree->addTopLevelItem(itemData);
    initVariable(itemData);

    QTreeWidgetItem *itemExpr = createFolder("计算");
    m_tree->addTopLevelItem(itemExpr);
    initExpression(itemExpr);

    if(m_fileType != ProjectItem_scriptParamBinding)
    {
        QTreeWidgetItem *itemProcess = createFolder("过程");
        m_tree->addTopLevelItem(itemProcess);
        initProcess(itemProcess);
    }
    m_tree->expandAll();
}

void JZNodePanel::addViriable(QString name)
{
    JZNodeParam node_get;
    JZNodeSetParam node_set;
    //node_get.setParamId(name);
    //node_set.setParamId(name);

    createItem(&node_get);
    createItem(&node_set);
}

void JZNodePanel::addWidget(QString name)
{
    //QString name = widget->objectName();
    //Q_ASSERT(name.isEmpty());
}

QTreeWidgetItem *JZNodePanel::createFolder(QString name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);
    item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);
    return item;
}

QTreeWidgetItem *JZNodePanel::createItem(JZNode *node)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, node->name());
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
    item->setData(0, Qt::UserRole, formatNode(node));
    return item;
}

void JZNodePanel::initEvent(QTreeWidgetItem *root)
{    
    QTreeWidgetItem *itemWidget = createFolder("控件事件");
    root->addChild(itemWidget);
/*
    JZNodeEvent event;
    event.setEventType();
    itemWidget->add
*/
}

void JZNodePanel::initVariable(QTreeWidgetItem *root)
{
    JZNodeLiteral node_int,node_int64,node_string,node_double;
    node_int.setName("整数");
    node_int.setLiteral(0);
    node_int.setDataType(Type_int);

    node_int64.setName("64位整数");
    node_int64.setLiteral(0);
    node_int.setDataType(Type_int64);

    node_string.setName("字符串");
    node_string.setLiteral("");
    node_int.setDataType(Type_string);

    node_double.setName("浮点数");
    node_double.setLiteral(0.0);
    node_int.setDataType(Type_double);

    root->addChild(createItem(&node_int));
    //root->addChild(createItem(&node_int64));
    root->addChild(createItem(&node_string));
    root->addChild(createItem(&node_double));

    JZNodeParam get;
    JZNodeSetParam set;
    JZNodeSetParamData set_data;
    JZNodeCreate create;
    root->addChild(createItem(&get));
    if(m_fileType == ProjectItem_scriptParamBinding)
    {
        root->addChild(createItem(&set_data));
    }
    else
    {
        root->addChild(createItem(&set));
        root->addChild(createItem(&create));
    }
}


void JZNodePanel::initExpression(QTreeWidgetItem *root)
{    
    QTreeWidgetItem *item_op = createFolder("算子");

    for (int i = Node_add; i <= Node_expr; i++)
    {
        QTreeWidgetItem *sub = new QTreeWidgetItem();
        auto node = JZNodeFactory::instance()->createNode(i);
        sub->setText(0, node->name());
        sub->setFlags(sub->flags() | Qt::ItemIsDragEnabled);
        sub->setData(0, Qt::UserRole, formatNode(node));
        item_op->addChild(sub);
        delete node;
    }

    QTreeWidgetItem *item_func = createFolder("函数");
    initFunction(item_func,false);

    root->addChild(item_op);
    root->addChild(item_func);
}

void JZNodePanel::initFunction(QTreeWidgetItem *root,bool flow)
{
    QMap<QString,QTreeWidgetItem*> itemMap;

    auto funcs = JZNodeFunctionManager::instance()->functionList();
    for (int i = 0; i < funcs.size(); i++)
    {
        if(funcs[i]->isFlowFunction != flow)
            continue;

        QString function = funcs[i]->name;

        int index = function.indexOf(".");
        QString class_name,short_name;
        if(index != -1)
        {
            class_name = function.left(index);
            short_name = function.mid(index + 1);
        }
        else
        {
            class_name = "全局";
            short_name = function;
        }

        QTreeWidgetItem *item_class;
        if(!itemMap.contains(class_name))
        {
            item_class = new QTreeWidgetItem();
            item_class->setText(0,class_name);
            itemMap[class_name] = item_class;
            root->addChild(item_class);
        }
        else
        {
            item_class = itemMap[class_name];
        }

        QTreeWidgetItem *sub = new QTreeWidgetItem();
        sub->setText(0, short_name);
        sub->setFlags(sub->flags() | Qt::ItemIsDragEnabled);

        JZNodeFunction node_func;
        node_func.setName(function);
        node_func.setFunction(funcs[i]);

        sub->setData(0, Qt::UserRole, formatNode(&node_func));
        item_class->addChild(sub);
    }
}

void JZNodePanel::initProcess(QTreeWidgetItem *root)
{
    QTreeWidgetItem *item_process = new QTreeWidgetItem();
    item_process->setText(0,"基本过程");

    JZNodeFor node_for;
    JZNodeWhile node_while;
    JZNodeSequence node_seq;
    JZNodeBranch node_branch;
    JZNodeForEach node_each;
    JZNodeBreak node_break;
    JZNodeContinue node_continue;
    JZNodeExit node_exit;
    item_process->addChild(createItem(&node_for));
    item_process->addChild(createItem(&node_while));
    item_process->addChild(createItem(&node_seq));
    item_process->addChild(createItem(&node_branch));
    item_process->addChild(createItem(&node_each));
    item_process->addChild(createItem(&node_break));
    item_process->addChild(createItem(&node_continue));
    item_process->addChild(createItem(&node_exit));

    QTreeWidgetItem *item_func = createFolder("函数");
    initFunction(item_func,true);

    root->addChild(item_process);
    root->addChild(item_func);
}
