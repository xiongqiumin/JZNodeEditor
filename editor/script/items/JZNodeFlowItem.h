#ifndef JZNODE_FLOW_ITEM_H_
#define JZNODE_FLOW_ITEM_H_

#include "JZNodeGraphItem.h"

class JZNodeForItem : public JZNodeGraphItem
{
public:    
    JZNodeForItem(JZNode *node);
    ~JZNodeForItem();

protected:
    void onCompareOpChanged();

};

#endif
