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
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);

    m_tree = new JZNodeTreeWidget();
    m_tree->setColumnCount(1);
    m_tree->setHeaderHidden(true);
    m_tree->setDragEnabled(true);

    QTreeWidgetItem *itemData = new QTreeWidgetItem();
    itemData->setText(0, "数据源");
    m_tree->addTopLevelItem(itemData);

    QTreeWidgetItem *itemDisplay = new QTreeWidgetItem();
    itemDisplay->setText(0, "显示");
    m_tree->addTopLevelItem(itemDisplay);

    QTreeWidgetItem *itemExpr = new QTreeWidgetItem();
    itemExpr->setText(0, "计算");
    m_tree->addTopLevelItem(itemExpr);

    QTreeWidgetItem *itemFunc = new QTreeWidgetItem();
    itemFunc->setText(0, "函数");
    m_tree->addTopLevelItem(itemFunc);

    layout->addWidget(m_tree);
    setLayout(layout);

    initVariable();
    initDisplay();
    initExpression();
    initFunction();

    m_tree->expandAll();
}

JZNodePanel::~JZNodePanel()
{
}

void JZNodePanel::initVariable()
{
    JZNodeLiteral source;
    source.setName("数据");

    auto item_var = m_tree->topLevelItem(0);
    QTreeWidgetItem *sub = new QTreeWidgetItem();
    sub->setText(0, "数据");
    sub->setFlags(sub->flags() | Qt::ItemIsDragEnabled);
    sub->setData(0, Qt::UserRole, formatNode(&source));

    item_var->addChild(sub);
}

void JZNodePanel::initDisplay()
{
    JZNodePrint source;
    source.setName("显示");            

    auto item_disp = m_tree->topLevelItem(1);
    QTreeWidgetItem *sub = new QTreeWidgetItem();
    sub->setText(0, "显示");
    sub->setFlags(sub->flags() | Qt::ItemIsDragEnabled);
    sub->setData(0, Qt::UserRole, formatNode(&source));

    item_disp->addChild(sub);
}

void JZNodePanel::initExpression()
{
    auto item_func = m_tree->topLevelItem(2);

    QStringList opList = QStringList{"+","-","*","%","%","==","!=",">",">=","<","<=","&&","||","^"};
    for (int i = 0; i < opList.size(); i++)
    {
        QTreeWidgetItem *sub = new QTreeWidgetItem();
        auto node = JZNodeFactory::instance()->createNode(Node_add + i);
        sub->setText(0, node->name());
        sub->setFlags(sub->flags() | Qt::ItemIsDragEnabled);

        sub->setData(0, Qt::UserRole, formatNode(node));
        item_func->addChild(sub);
        delete node;
    }
}

void JZNodePanel::initFunction()
{
    auto item_func = m_tree->topLevelItem(3);
    auto funcs = JZNodeFunctionManager::instance()->functionList();
    for (int i = 0; i < funcs.size(); i++)
    {
        QTreeWidgetItem *sub = new QTreeWidgetItem();
        sub->setText(0, funcs[i]->name);
        sub->setFlags(sub->flags() | Qt::ItemIsDragEnabled);

        JZNodeFunction node_func;
        node_func.setName(funcs[i]->name);
        node_func.setFunction(funcs[i],false);

        sub->setData(0, Qt::UserRole, formatNode(&node_func));
        item_func->addChild(sub);
    }
}
