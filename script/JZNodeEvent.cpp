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

//JZNodeSignalConnect
JZNodeSignalConnect::JZNodeSignalConnect()
{
    addFlowIn();
    addFlowOut();
    
    int in1 = addParamIn("sender", Pin_dispName);
    int in2 = addParamIn("signal", Pin_dispName | Pin_literal);
    int in3 = addParamIn("receiver", Pin_dispName);
    int in4 = addParamIn("slot", Pin_dispName | Pin_literal);

    setPinType(in1, { Type_object });
    setPinType(in2, { Type_function });
    setPinType(in3, { Type_object });
    setPinType(in4, { Type_function });
}

JZNodeSignalConnect::~JZNodeSignalConnect()
{

}

bool JZNodeSignalConnect::compiler(JZNodeCompiler *c, QString &error)
{
    if(!c->addFlowInput(m_id,error))
        return false;

    JZFunctionPointer sig,slot;
    sig = c->pinLiteral(m_id,paramIn(1)).value<JZFunctionPointer>();
    slot = c->pinLiteral(m_id,paramIn(3)).value<JZFunctionPointer>();

    auto sig_func = JZNodeObjectManager::instance()->single(sig.functionName);
    auto slot_func = JZNodeFunctionManager::instance()->function(slot.functionName);
    if(!sig_func)
    {
        error = "no signal " + sig.functionName;
        return false;
    }
    if(!slot_func)
    {
        error = "no slot " + slot.functionName;
        return false;
    }
    if(!JZNodeType::sigSlotTypeMatch(sig_func,slot_func))
    {
        error = "signal slot not match " + sig_func->delcare() + "," + slot_func->delcare();
        return false;
    }

    int send_id = c->paramId(m_id,paramIn(0));
    int recv_id = c->paramId(m_id,paramIn(2));
    QList<JZNodeIRParam> in,out;
    in << irId(send_id) << irLiteral(QVariant::fromValue(sig)) << irId(recv_id) << irLiteral(QVariant::fromValue(slot));
    c->addCall("connect",in,out);
    c->addJumpNode(flowOut());
    
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