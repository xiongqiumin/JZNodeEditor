#ifndef JZ_LOG_NODE_H_
#define JZ_LOG_NODE_H_

#include "../JZModuleDefine.h"
#include "JZNode.h"

enum {
    Node_logEvent = Module_LogNode,
};

class JZNodeLogEvent :  public JZNode
{
public:
    JZNodeLogEvent();
};

#endif