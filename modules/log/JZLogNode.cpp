#include "JZLogNode.h"

JZNodeLogEvent::JZNodeLogEvent()
{    
    m_type = Node_logEvent;

    addFlowIn();
    addFlowOut();

    auto in = addParamIn("text");
    setPinTypeString(in);
}