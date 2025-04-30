#ifndef JZNODE_DISPLAY_ITEM_H_
#define JZNODE_DISPLAY_ITEM_H_

#include "JZNodeGraphItem.h"

//JZNodeDisplayItem
class JZNodeDisplayItem : public JZNodeGraphItem
{
public:
    JZNodeDisplayItem();

    void setValue(int pin,QVariantPtr *ref);

protected:
    virtual void updatePin() override;
    void onAddClicked();

    BlockPtr m_addBlock;
};


#endif
