#ifndef JZNODE_UI_FILE_H_
#define JZNODE_UI_FILE_H_

#include <QDomElement>
#include "JZProjectItem.h"
#include "JZNode.h"
#include "JZNodeObject.h"

//JZUiExtItem
class JZUiExtItem : public JZProjectItem
{
public:    
    JZUiExtItem();
    virtual ~JZUiExtItem();    
    
protected:
    virtual void saveToStream(QDataStream &s) const override;
    virtual bool loadFromStream(QDataStream &s) override;
};



#endif
