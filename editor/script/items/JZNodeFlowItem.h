#ifndef JZNODE_FLOW_ITEM_H_
#define JZNODE_FLOW_ITEM_H_

#include "JZNodeGraphItem.h"

class JZNodeForItem : public JZNodeGraphItem
{
public:    
    JZNodeForItem();
    ~JZNodeForItem();

protected:
    virtual void updatePin() override;
    void onCompareOpChanged(int op);

    BlockPtr m_opBlock;
};

class JZNodeIfItem : public JZNodeGraphItem
{
public:
    JZNodeIfItem();
    ~JZNodeIfItem();

protected:    
    virtual void updatePin() override;
    void onAddClicked();
    void onElseClicked();

    BlockPtr m_addCond;
    BlockPtr m_addElse;
};

class JZNodeSwitchItem : public JZNodeGraphItem
{
public:
    JZNodeSwitchItem();
    ~JZNodeSwitchItem();

protected:
    virtual void updatePin() override;
    void onAddClicked();

    BlockPtr m_addSwitch;
    BlockPtr m_addDefault;
};

#endif
