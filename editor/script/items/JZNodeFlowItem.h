#ifndef JZNODE_FLOW_ITEM_H_
#define JZNODE_FLOW_ITEM_H_

#include "JZNodeGraphItem.h"

//JZNodeForItem
class JZNodeForItem : public JZNodeGraphItem
{
public:    
    JZNodeForItem(JZNode *node);
    ~JZNodeForItem();

protected:
    virtual void updatePin() override;
    void onCompareOpChanged(int op);

    BlockPtr m_opBlock;
};

//JZNodeForEachItem
class JZNodeForeachItem : public JZNodeGraphItem
{
public:
    JZNodeForeachItem(JZNode *node);
    ~JZNodeForeachItem();

protected:
    virtual void updatePin() override;    
};

//JZNodeIfItem
class JZNodeIfItem : public JZNodeGraphItem
{
public:
    JZNodeIfItem(JZNode *node);
    ~JZNodeIfItem();

protected:    
    virtual void updatePin() override;
    void onAddClicked();
    void onElseClicked();

    BlockPtr m_addCond;
    BlockPtr m_addElse;
};

//JZNodeSwitchItem
class JZNodeSwitchItem : public JZNodeGraphItem
{
public:
    JZNodeSwitchItem(JZNode *node);
    ~JZNodeSwitchItem();

protected:
    virtual void updatePin() override;
    void onAddClicked();

    BlockPtr m_addSwitch;
    BlockPtr m_addDefault;
};

//JZNodeTryCatchItem
class JZNodeTryCatchItem : public JZNodeGraphItem
{
public:
    JZNodeTryCatchItem(JZNode *node);

protected:
    virtual void updatePin() override;
};

#endif
