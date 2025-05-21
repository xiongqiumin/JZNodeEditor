#ifndef JZNODE_UI_BASE_ITEM_H_
#define JZNODE_UI_BASE_ITEM_H_

#include "JZProjectItem.h"
#include "JZNodeObject.h"

//JZUiBaseItem
class JZUiBaseItem : public JZProjectItem
{
public:    
    JZUiBaseItem();

    virtual JZNodeObjectWidgetDefine define() = 0;
    
    const JZParamDefine *widgetVariable(QString name);
    QList<JZParamDefine> widgets();
    
protected:
    QList<JZParamDefine> m_widgets;
};



#endif
