#ifndef JZ_SCRIPT_ITEM_VISTOR_H_
#define JZ_SCRIPT_ITEM_VISTOR_H_

#include "JZScriptItem.h"
#include "JZNodeFlow.h"
#include "JZNodeFunction.h"
#include "JZNodeEvent.h"
#include "JZNodeValue.h"

class JZScriptItemVistor
{
public:    
    JZScriptItemVistor();
    virtual ~JZScriptItemVistor();

    void visitorScript(JZScriptItem *item);
    void visitor(JZNode *node);

protected:
    virtual void visitorSelf(JZNode *node);

    QList<JZNode*> getPinNode(JZNode *node,int pin);
    QList<JZNode*> dataInputNode(JZNode *node);

    JZNode *nextFlowNode(JZNode *node,int id);
    JZNode *flowInputNode(JZNode *node,int id);

    QList<JZNode*> allDataInputNode(JZNode *node);

    JZScriptItem *m_script;

};

#endif