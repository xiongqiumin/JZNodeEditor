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
    auto &func = m_file->function();
    for(int i = 0; i < func.paramIn.size(); i++)
    {
        if(func.paramIn[i].dataType() == Type_none)
        {
            error = "输入" + func.paramIn[i].name + "类型错误," + func.paramIn[i].type;
            return false;
        }
    }
    for(int i = 0; i < func.paramOut.size(); i++)
    {
        if(func.paramOut[i].dataType() == Type_none)
        {
            error = "输入" + func.paramOut[i].name + "类型错误," + func.paramIn[i].type;
            return false;
        }
    }

    c->addFunctionAlloc(m_file->function());
    c->addNodeDebug(m_id);
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeSignalConnect
JZNodeSignalConnect::JZNodeSignalConnect()
{
    m_type = Node_signalConnect;
    m_name = "connect";
    
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

    QString sig = c->pinLiteral(m_id,paramIn(1));
    QString slot = c->pinLiteral(m_id,paramIn(3));

    auto sig_func = JZNodeObjectManager::instance()->signal(sig);
    auto slot_func = JZNodeFunctionManager::instance()->function(slot);
    if(!sig_func)
    {
        error = "no signal " + sig;
        return false;
    }
    if(!slot_func)
    {
        error = "no slot " + slot;
        return false;
    }
    if(!JZNodeType::sigSlotTypeMatch(sig_func,slot_func))
    {
        error = "signal slot not match " + sig_func->fullName() + "," + slot_func->delcare();
        return false;
    }

    JZFunctionPointer sig_ptr;
    sig_ptr.functionName = sig;
    JZFunctionPointer slot_ptr;
    slot_ptr.functionName = slot;

    int send_id = c->paramId(m_id,paramIn(0));
    int recv_id = c->paramId(m_id,paramIn(2));
    QList<JZNodeIRParam> in,out;
    in << irId(send_id) << irLiteral(QVariant::fromValue(sig_ptr)) << irId(recv_id) << irLiteral(QVariant::fromValue(slot_ptr));
    c->addCall("connect",in,out);
    c->addJumpNode(flowOut());
    
    return true;
}

//JZNodeSignalDisconnect
JZNodeSignalDisconnect::JZNodeSignalDisconnect()
{
}

JZNodeSignalDisconnect::~JZNodeSignalDisconnect()
{
}

bool JZNodeSignalDisconnect::compiler(JZNodeCompiler *c, QString &error)
{
    return false;
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