#ifndef JZNODE_DISPLAY_ITEM_H_
#define JZNODE_DISPLAY_ITEM_H_

#include "JZNodeGraphItem.h"

//JZNodeDisplayItem
class JZImageLabel;
class JZNodeDisplayItem : public JZNodeGraphItem
{
public:
    JZNodeDisplayItem(JZNode *node);

    void setValue(int pin,QVariantPtr *ref);

protected:
    virtual void updatePin() override;    

    void onAddClicked();
    void onLabelExpand(JZImageLabel *label);

    QString getInputType(int id);

    BlockPtr m_addBlock;
};


#endif
