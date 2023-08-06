#include "JZNodeFunction.h"
#include "JZNodeCompiler.h"

JZNodeFunctionStart::JZNodeFunctionStart()
{
    m_name = "Start";
    m_type = Node_functionStart;
    addFlowOut();   
}

JZNodeFunctionStart::~JZNodeFunctionStart()
{
    
}

bool JZNodeFunctionStart::compiler(JZNodeCompiler *c,QString &error)
{
    c->allocFunctionVariable();
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeFunction
JZNodeFunction::JZNodeFunction()
{
    m_type = Node_function;
}

JZNodeFunction::~JZNodeFunction()
{

}

void JZNodeFunction::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
    s << m_functionName;
}

void JZNodeFunction::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
    s >> m_functionName;
}

void JZNodeFunction::setFunction(const FunctionDefine *define)
{
    Q_ASSERT(define);
    if(define->isFlowFunction)
    {
        addFlowIn();
        addFlowOut();
    }
    for(int i = 0; i < define->paramIn.size(); i++)
    {
        JZNodePin pin;
        pin.setName(define->paramIn[i].name);
        pin.setFlag(Prop_param | Prop_in | Prop_dispName);
        pin.setDataType({define->paramIn[i].dataType});
        addProp(pin);
    }
    for(int i = 0; i < define->paramOut.size(); i++)
    {
        JZNodePin pin;
        pin.setName(define->paramOut[i].name);
        pin.setFlag(Prop_param | Prop_out | Prop_dispName);
        pin.setDataType({define->paramOut[i].dataType});
        addProp(pin);
    }
    m_functionName = define->fullName();    
    setName(m_functionName);
}

QString JZNodeFunction::function() const
{
    return m_functionName;
}

bool JZNodeFunction::compiler(JZNodeCompiler *c,QString &error)
{
    bool input_ret = false;
    if(isFlowNode())
        input_ret = c->addFlowInput(m_id,error);
    else
        input_ret = c->addDataInput(m_id,error);
    if(!input_ret)        
        return false;
        
    QVector<int> in_list = propInList(Prop_param);
    QVector<int> out_list = propOutList(Prop_param);
    for(int i = 0; i < in_list.size(); i++)
    {
        int id = c->paramId(m_id,in_list[i]);
        c->addSetVariable(irId(Reg_Call+i),irId(id));
    }

    JZNodeIRCall *call = new JZNodeIRCall();
    call->function = irLiteral(m_functionName);
    c->addStatement(JZNodeIRPtr(call));

    for(int i = 0; i < out_list.size(); i++)
    {
        int id = c->paramId(m_id,out_list[i]);
        c->addSetVariable(irId(id),irId(Reg_Call+i));
    }

    if (isFlowNode())
    {
        c->addFlowOutput(m_id);
        c->addJumpNode(flowOut());
    }
    return true;
}
