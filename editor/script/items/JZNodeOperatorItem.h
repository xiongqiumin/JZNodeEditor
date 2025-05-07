#ifndef JZNODE_OPERATOR_ITEM_H_
#define JZNODE_OPERATOR_ITEM_H_

#include "JZNodeGraphItem.h"


class JZNodeOperatorItem : public JZNodeGraphItem
{
public:
    JZNodeOperatorItem(JZNode *node);
    ~JZNodeOperatorItem();

protected:
    virtual void updatePin() override;
    void onBtnAddClicked();

    BlockPtr m_addBlock;
};


class JZNodeExpressionItem : public JZNodeGraphItem
{
public:
    JZNodeExpressionItem(JZNode *node);
    ~JZNodeExpressionItem();

protected:
    void onBtnSetClicked();

    BlockPtr m_setBlock;
};

#endif