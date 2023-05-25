#include "JZNodeFunction.h"
#include "JZNodeCompiler.h"


JZNodeFunctionStart::JZNodeFunctionStart()
{
    m_type = Node_functionStart;
    addFlowOut();   
}

JZNodeFunctionStart::~JZNodeFunctionStart()
{
    
}

bool JZNodeFunctionStart::compiler(JZNodeCompiler *c,QString &error)
{
    c->addFlowOutput(0);
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

void JZNodeFunction::setFunction(const FunctionDefine *define,bool flowFunction)
{
    if(flowFunction)
    {
        addFlowIn();
        addFlowOut();
    }
    for(int i = 0; i < define->paramIn.size(); i++)
        addProp(define->paramIn[i]);
    for(int i = 0; i < define->paramOut.size(); i++)
        addProp(define->paramOut[i]);
    m_functionName = define->name;
}

QString JZNodeFunction::function() const
{
    return m_functionName;
}

bool JZNodeFunction::compiler(JZNodeCompiler *c,QString &error)
{
    if(isFlowNode())
        c->addFlowInput(m_id);
    else
        c->addDataInput(m_id);

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

    if(isFlowNode())
        c->addJumpNode(flowOut());

    return true;
}
