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
    virtual void visitorSelf(JZNode *node) = 0;
    
    QList<JZNode*> dataInputNode(JZNode *node);
    QList<JZNode*> dataInputNodeRecursively(JZNode *node);

    JZNode *nextFlowNode(JZNode *node,int id);    

    JZScriptItem *m_script;
    QList<JZNode*> m_hasVistorNode;
};

#endif