#include "JZNodeFunction.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"

JZNodeFunctionStart::JZNodeFunctionStart()
{
    m_name = "Start";
    m_type = Node_functionStart;
    setFlag(Node_pinNoRemove);
    addFlowOut();   
}

JZNodeFunctionStart::~JZNodeFunctionStart()
{
    
}

bool JZNodeFunctionStart::compiler(JZNodeCompiler *c,QString &error)
{
    c->addFunctionAlloc(m_file->function());
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
    s  >> m_functionName;
}

void JZNodeFunction::setFunction(const JZFunctionDefine *define)
{
    Q_ASSERT(define);

    m_functionName = define->fullName();
    setName(m_functionName);

    if(define->isFlowFunction)
    {
        addFlowIn();
        addFlowOut();
    }
    auto edit = JZNodeFunctionManager::instance()->editFunction(define->fullName());
    if (edit)
        addButtonIn("edit");

    for(int i = 0; i < define->paramIn.size(); i++)
    {
        JZNodePin pin;
        pin.setName(define->paramIn[i].name);
        pin.setFlag(Pin_param | Pin_in | Pin_dispName);
        pin.setDataType({define->paramIn[i].dataType() });
        if(JZNodeType::isBaseOrEnum(define->paramIn[i].dataType())
            || define->paramIn[i].dataType() == Type_any)
            pin.setFlag(pin.flag() | Pin_dispValue | Pin_editValue);
        addPin(pin);
    }
    for(int i = 0; i < define->paramOut.size(); i++)
    {
        JZNodePin pin;
        pin.setName(define->paramOut[i].name);
        pin.setFlag(Pin_param | Pin_out | Pin_dispName);
        pin.setDataType({define->paramOut[i].dataType()});
        addPin(pin);
    }            
}

QString JZNodeFunction::function() const
{
    return m_functionName;
}

bool JZNodeFunction::pinClicked(int id)
{
    Q_UNUSED(id);
    auto edit = JZNodeFunctionManager::instance()->editFunction(m_functionName);    
    return edit(this);
}

bool JZNodeFunction::compiler(JZNodeCompiler *c,QString &error)
{
    auto def = c->function(m_functionName);
    if (!def)
    {
        error = "函数不存在";
        return false;
    }

    QVector<int> in_list = pinInList(Pin_param);
    QVector<int> out_list = pinOutList(Pin_param);
    if (def->paramIn.size() != in_list.size())
    {
        error = QString("函数不接受%1个输入").arg(in_list.size());
        return false;
    }
    if (def->paramOut.size() != out_list.size())
    {
        error = QString("函数不接受%1个输出").arg(out_list.size());
        return false;
    }

    bool input_ret = false;
    if(isFlowNode())
        input_ret = c->addFlowInput(m_id,error);
    else
        input_ret = c->addDataInput(m_id,error);
    if(!input_ret)        
        return false;
            
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
