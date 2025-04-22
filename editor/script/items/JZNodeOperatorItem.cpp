#include <QPushButton>
#include "JZNodeOperatorItem.h"
#include "JZNodeOperator.h"
#include "JZNodeFactory.h"
#include "JZNodeExpression.h"

JZNodeOperatorItem::JZNodeOperatorItem()
{
}

JZNodeOperatorItem::~JZNodeOperatorItem()
{
}

void JZNodeOperatorItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    if (m_addBlock)
    {
        QPushButton *btn = new QPushButton("Add");
        m_addBlock = fromWidget(btn,true);
        btn->connect(btn, &QPushButton::clicked, [this] {
            this->onBtnAddClicked();
        });
    }
}

void JZNodeOperatorItem::onBtnAddClicked()
{
    JZNodeOperator *node = (JZNodeOperator*)m_node;

    QByteArray buffer = JZNodeFactory::instance()->saveNode(node);
    node->addInput();
    notifyPropChanged(buffer);
}

//JZNodeExpressionItem
JZNodeExpressionItem::JZNodeExpressionItem()
{
}

JZNodeExpressionItem::~JZNodeExpressionItem()
{
}

void JZNodeExpressionItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    if (m_setBlock)
    {
        QPushButton *btn = new QPushButton("Setting");
        m_setBlock = fromWidget(btn, true);
        btn->connect(btn, &QPushButton::clicked, [this] {
            this->onBtnSetClicked();
        });
    }
}

void JZNodeExpressionItem::onBtnSetClicked()
{
    JZNodeExpression *node = (JZNodeExpression*)m_node;
    QString expr;

    QByteArray buffer = JZNodeFactory::instance()->saveNode(node);
    node->setExpr(expr);
    notifyPropChanged(buffer);
}