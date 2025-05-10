#include <QComboBox>
#include <QPushButton>
#include "JZNodeFlowItem.h"
#include "JZNodeFlow.h"
#include "JZNodeFactory.h"

JZNodeForItem::JZNodeForItem(JZNode *node)
    :JZNodeGraphItem(node)
{
}

JZNodeForItem::~JZNodeForItem()
{
}

void JZNodeForItem::updatePin()
{
    JZNodeGraphItem::updatePin();
    m_blocks[m_node->paramIn(0)]->pri = Pri_user + 1;
    m_blocks[m_node->paramIn(1)]->pri = Pri_user + 2;
    m_blocks[m_node->paramIn(2)]->pri = Pri_user + 4;

    JZNodeFor *node_for = dynamic_cast<JZNodeFor*>(m_node);
    if (!m_opBlock)
    {                
        auto box = new QComboBox();
        box->addItem("<", OP_lt);
        box->addItem("<=", OP_le);
        box->addItem(">", OP_gt);
        box->addItem(">=", OP_ge);
        box->addItem("==", OP_eq);
        box->addItem("!=", OP_ne);        
        box->connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, box](){
            int op = box->currentData().toInt();
            this->onCompareOpChanged(op);
        });

        m_opBlock = createWidgetBlock(box, true);
        m_opBlock->name = "op";
        m_opBlock->pri = Pri_user + 3;
    }
    
    auto pin_list = m_node->paramOutList();
    m_blocks[pin_list[0]]->pri = Pri_user + 0;    

    int out_flow = m_node->flowOut();
    m_blocks[out_flow]->pri = Pri_user + 1;

    QComboBox *box_op = qobject_cast<QComboBox*>(m_opBlock->widget);
    box_op->blockSignals(true);
    int index = box_op->findData(node_for->op());
    box_op->setCurrentIndex(index);
    box_op->blockSignals(false);
}

void JZNodeForItem::onCompareOpChanged(int op)
{    
    QByteArray oldValue = saveNode();

    JZNodeFor *node_for = (JZNodeFor*)m_node;
    node_for->setOp((JZNodeIRType)op);
    notifyPropChanged(oldValue); 
}

//JZNodeForeachItem
JZNodeForeachItem::JZNodeForeachItem(JZNode *node)
    :JZNodeGraphItem(node)
{
}

JZNodeForeachItem::~JZNodeForeachItem()
{

}

void JZNodeForeachItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    auto pin_list = m_node->paramOutList();
    m_blocks[pin_list[0]]->pri = Pri_user + 0;
    m_blocks[pin_list[1]]->pri = Pri_user + 1;

    int out_flow = m_node->flowOut();
    m_blocks[out_flow]->pri = Pri_user + 2;
}

//JZNodeIfItem
JZNodeIfItem::JZNodeIfItem(JZNode *node)
    :JZNodeGraphItem(node)
{
}
JZNodeIfItem::~JZNodeIfItem()
{
}

void JZNodeIfItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    if (!m_addCond)
    {
        QPushButton *btnAdd = new QPushButton("Add Cond");
        QPushButton *btnElse = new QPushButton("Add Else");
        btnAdd->connect(btnAdd, &QPushButton::clicked, [this] {
            this->onAddClicked();
        });
        btnElse->connect(btnElse, &QPushButton::clicked, [this] {
            this->onElseClicked();
        });
        
        m_addCond = createWidgetBlock(btnAdd,true);
        m_addCond->pri = 8;

        m_addElse = createWidgetBlock(btnElse, true);
        m_addElse->pri = 9;
    }
    JZNodeIf *node_if = (JZNodeIf *)m_node;
    auto btn_else = qobject_cast<QPushButton*>(m_addElse->widget);
    if (node_if->hasElse())
        btn_else->setText("Remove Else");
    else
        btn_else->setText("Add Else");

    QList<int> cond_list = m_node->paramInList();
    QList<int> flow_list = m_node->subFlowList();
    for (int i = 0; i < cond_list.size(); i++)
    {
        m_blocks[cond_list[i]]->name = "Cond" + QString::number(i);
        m_blocks[flow_list[i]]->name = "Cond" + QString::number(i);
    }
    if (node_if->hasElse())
        m_blocks[flow_list.back()]->name = "Else";
}

void JZNodeIfItem::onAddClicked()
{
    JZNodeIf *node_if = (JZNodeIf *)m_node;

    QByteArray oldValue = saveNode();
    node_if->addCondPin();
    notifyPropChanged(oldValue);
}

void JZNodeIfItem::onElseClicked()
{
    JZNodeIf *node_if = (JZNodeIf *)m_node;

    QByteArray oldValue = saveNode();
    if (!node_if->hasElse())
        node_if->addElsePin();
    else
        node_if->removeElse();
    notifyPropChanged(oldValue);
}

//JZNodeSwitchItem
JZNodeSwitchItem::JZNodeSwitchItem(JZNode *node)
    :JZNodeGraphItem(node)
{

}

JZNodeSwitchItem::~JZNodeSwitchItem()
{

}


void JZNodeSwitchItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    if (!m_addSwitch)
    {
        QPushButton *btnAdd = new QPushButton("Add Cond");        
        btnAdd->connect(btnAdd, &QPushButton::clicked, [this] {
            this->onAddClicked();
        });
        m_addSwitch = createWidgetBlock(btnAdd, true);
        m_addSwitch->pri = 8;
    }
}

void JZNodeSwitchItem::onAddClicked()
{
    JZNodeSwitch *node_switch = (JZNodeSwitch *)m_node;

    QByteArray oldValue = saveNode();
    node_switch->addCase();
    notifyPropChanged(oldValue);
}


//JZNodeTryCatchItem
JZNodeTryCatchItem::JZNodeTryCatchItem(JZNode *node)
    :JZNodeGraphItem(node)
{
}

void JZNodeTryCatchItem::updatePin()
{
    JZNodeGraphItem::updatePin();

    auto pin_list = m_node->paramOutList();
    m_blocks[pin_list[0]]->pri = Pri_user + 0;

    int out_flow = m_node->flowOut();
    m_blocks[out_flow]->pri = Pri_user + 1;
}