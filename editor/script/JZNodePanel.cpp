#include "JZNodePanel.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include "JZNodeFunctionManager.h"
#include "JZNodeValue.h"
#include "JZNodeFactory.h"

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

    QTreeWidgetItem *itemEvent = createFolder("事件");
    m_tree->addTopLevelItem(itemEvent);

    QTreeWidgetItem *itemData = createFolder("数据");
    m_tree->addTopLevelItem(itemData);

    QTreeWidgetItem *itemExpr = createFolder("计算");
    m_tree->addTopLevelItem(itemExpr);

    QTreeWidgetItem *itemFunc = createFolder("过程");
    m_tree->addTopLevelItem(itemFunc);

    initEvent(itemEvent);
    initVariable(itemData);
    initExpression(itemExpr);
    initFunction(itemFunc);

    m_tree->expandAll();
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
    JZNodeLiteral node_int,node_string,node_double;
    node_int.setName("整数");
    node_int.setLiteral(0);

    node_string.setName("字符串");
    node_string.setLiteral("");

    node_double.setName("浮点数");
    node_double.setLiteral(0.0);

    QTreeWidgetItem *item_literal = createFolder("常量");
    item_literal->addChild(createItem(&node_int));
    item_literal->addChild(createItem(&node_string));
    item_literal->addChild(createItem(&node_double));

    QTreeWidgetItem *item_get = createFolder("变量引用");

    QTreeWidgetItem *item_set = createFolder("变量设置");

/*
    sub->setFlags(sub->flags() | Qt::ItemIsDragEnabled);
    sub->setData(0, Qt::UserRole, formatNode(&node_literal));

    root->addChild(sub);

    JZNodePrint source;
    source.setName("显示");            

    auto item_disp = m_tree->topLevelItem(1);
    QTreeWidgetItem *sub = new QTreeWidgetItem();
    sub->setText(0, "显示");
    sub->setFlags(sub->flags() | Qt::ItemIsDragEnabled);
    sub->setData(0, Qt::UserRole, formatNode(&source));
*/
    root->addChild(item_literal);
    root->addChild(item_get);
    root->addChild(item_set);
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
    auto funcs = JZNodeFunctionManager::instance()->functionList();
    for (int i = 0; i < funcs.size(); i++)
    {
        if(funcs[i]->name.contains("."))
            continue;

        QTreeWidgetItem *sub = new QTreeWidgetItem();
        sub->setText(0, funcs[i]->name);
        sub->setFlags(sub->flags() | Qt::ItemIsDragEnabled);

        JZNodeFunction node_func;
        node_func.setName(funcs[i]->name);
        node_func.setFunction(funcs[i],false);

        sub->setData(0, Qt::UserRole, formatNode(&node_func));
        item_func->addChild(sub);
    }

    root->addChild(item_op);
}

void JZNodePanel::initFunction(QTreeWidgetItem *root)
{

}
