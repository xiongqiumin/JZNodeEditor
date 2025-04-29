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

    void visitorScript(const JZScriptItem *item);
    void visitor(const JZNode *node);

protected:
    virtual void visitorSelf(const JZNode *node);

    QList<const JZNode*> getPinNode(const JZNode *node,int pin);
    QList<const JZNode*> dataInputNode(const JZNode *node);

    const JZNode *nextFlowNode(const JZNode *node,int id);
    const JZNode *flowInputNode(const JZNode *node,int id);

    QList<const JZNode*> allDataInputNode(const JZNode *node);

    const JZScriptItem *m_script;

};

#endif