#ifndef JZNODE_VALUE_ITEM_H_
#define JZNODE_VALUE_ITEM_H_

#include "JZNodeGraphItem.h"

//JZNodeLiteralItem
class JZNodeLiteralItem : public JZNodeGraphItem
{
public:
    JZNodeLiteralItem();
};

//JZNodeParamItem
class JZNodeParamItem : public JZNodeGraphItem
{
public:
    JZNodeParamItem();
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
};

//JZNodeFlagItem
class JZNodeFlagItem : public JZNodeGraphItem
{
public:
    JZNodeFlagItem();
};

#endif
