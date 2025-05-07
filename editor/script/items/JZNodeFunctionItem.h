#ifndef JZNODE_FUNCTION_ITEM_H_
#define JZNODE_FUNCTION_ITEM_H_

#include "JZNodeGraphItem.h"

//JZNodeFunctionItem
class JZNodeFunctionItem : public JZNodeGraphItem
{
public:
    JZNodeFunctionItem(JZNode *node);

    virtual void updatePin();
};


#endif