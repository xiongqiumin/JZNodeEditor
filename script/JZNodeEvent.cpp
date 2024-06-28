#include "JZNodeEvent.h"
#include "JZEvent.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"

// JZNodeEvent
JZNodeEvent::JZNodeEvent()
{
    m_type = Node_none;

    addFlowOut();
}

JZNodeEvent::~JZNodeEvent()
{

}

//JZNodeFunctionStart
JZNodeFunctionStart::JZNodeFunctionStart()
{
    m_name = "Start";
    m_type = Node_functionStart;
    setFlag(NodeProp_noRemove);
}

JZNodeFunctionStart::~JZNodeFunctionStart()
{

}

JZFunctionDefine JZNodeFunctionStart::function()
{
    if (!m_file)
        return JZFunctionDefine();

    return m_file->function();
}

bool JZNodeFunctionStart::compiler(JZNodeCompiler *c, QString &error)
{
    c->addFunctionAlloc(m_file->function());
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeSingleConnect
JZNodeSingleConnect::JZNodeSingleConnect()
{
    addFlowIn();
    addFlowOut();
    
    int in1 = addParamIn("sender", Pin_dispName);
    int in2 = addParamIn("signal", Pin_dispName | Pin_literal);
    int in3 = addParamIn("receiver", Pin_dispName);
    int in4 = addParamIn("slot", Pin_dispName | Pin_literal);

    setPinType(in1, { Type_object });
    setPinTypeString(in2);
    setPinType(in3, { Type_object });
    setPinTypeString(in4);
}

JZNodeSingleConnect::~JZNodeSingleConnect()
{

}

bool JZNodeSingleConnect::compiler(JZNodeCompiler *compiler, QString &error)
{
    return true;
}

//JZNodeParamChangedEvent
JZNodeParamChangedEvent::JZNodeParamChangedEvent()
{
    m_name = "ParamChanged";
    m_type = Node_paramChangedEvent;

    addParamIn("", Pin_dispValue | Pin_editValue);
}

JZNodeParamChangedEvent::~JZNodeParamChangedEvent()
{

}

void JZNodeParamChangedEvent::setVariable(const QString &name)
{

}

QString JZNodeParamChangedEvent::variable() const
{
    return QString();
}

JZFunctionDefine JZNodeParamChangedEvent::function()
{
    JZFunctionDefine def;
    return def;
}

bool JZNodeParamChangedEvent::compiler(JZNodeCompiler *compiler, QString &error)
{
    return false;
}