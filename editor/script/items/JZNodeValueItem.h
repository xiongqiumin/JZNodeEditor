#ifndef JZNODE_VALUE_ITEM_H_
#define JZNODE_VALUE_ITEM_H_

#include "JZNodeGraphItem.h"

//JZNodeLiteralItem
class JZNodeLiteralItem : public JZNodeGraphItem
{
public:
    JZNodeLiteralItem();

protected:
    virtual void updatePin() override;
};

//JZNodeParamItem
class JZNodeParamItem : public JZNodeGraphItem
{
public:
    JZNodeParamItem();

    virtual void updatePin() override;
};

//JZNodeSetParamItem
class JZNodeSetParamItem : public JZNodeGraphItem
{
public:
    JZNodeSetParamItem();
};

//JZNodeEnumItem
class JZNodeEnumItem : public JZNodeGraphItem
{
public:
    JZNodeEnumItem();

    virtual void updatePin() override;
};

//JZNodeFlagItem
class JZNodeFlagItem : public JZNodeGraphItem
{
public:
    JZNodeFlagItem();

    virtual void updatePin() override;
};

#endif
