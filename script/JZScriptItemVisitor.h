#ifndef JZ_SCRIPT_ITEM_VISTOR_H_
#define JZ_SCRIPT_ITEM_VISTOR_H_

#include "JZScriptItem.h"
#include "JZNodeFlow.h"
#include "JZNodeFunction.h"
#include "JZNodeEvent.h"
#include "JZNodeValue.h"

class JZScriptItemVisitor
{
public:    
    JZScriptItemVisitor(JZScriptItem *item = nullptr);
    virtual ~JZScriptItemVisitor();

    void setScript(JZScriptItem *item);        
    void visitor();

    QList<JZNodePin*> inputPin(int node_id, int pin_id);

    QList<JZNode*> dataInputNode(JZNode *node);
    QList<JZNode*> dataInputNode(JZNode* node, int pin_id);
    QList<JZNode*> dataInputNodeRecursively(JZNode *node);

    JZNode *nextFlowNode(JZNode *node, int id);    
    JZNode *prevFlowNode(JZNode *node);

protected:
    void visitorNode(JZNode *node);
    virtual void visitorSelf(JZNode *node);
        
    JZScriptItem *m_script;
    QList<JZNode*> m_hasVistorNode;
};

#endif