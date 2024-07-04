#include "JZNodeFunction.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"

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

void JZNodeFunction::setFunction(const QString &name)
{
    auto def = JZNodeFunctionManager::instance()->function(name);
    setFunction(def);
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

    for(int i = 0; i < define->paramIn.size(); i++)
    {
        JZNodePin pin;
        pin.setName(define->paramIn[i].name);    
        pin.setFlag(Pin_param | Pin_in | Pin_dispName);
        pin.setDataType({define->paramIn[i].dataType() });
        if (JZNodeType::isBaseOrEnum(define->paramIn[i].dataType()))        
            pin.setFlag(pin.flag() | Pin_dispValue | Pin_editValue);        
        pin.setValue(define->paramIn[i].value);
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

JZFunctionDefine JZNodeFunction::functionDefine()
{
    JZFunctionDefine def;
    def.setFullName(m_functionName);
    def.isFlowFunction = isFlowNode();

    auto in_list = paramInList();
    for(int i = 0; i < in_list.size(); i++)
    {
        auto in = pin(in_list[i]);
        QString in_type = JZNodeType::typeToName(in->dataType()[0]);
        def.paramIn.push_back(JZParamDefine(in->name(),in_type));
    }

    auto out_list = paramOutList();
    for(int i = 0; i < out_list.size(); i++)
    {
        auto out = pin(out_list[i]);
        QString out_type = JZNodeType::typeToName(out->dataType()[0]);
        def.paramOut.push_back(JZParamDefine(out->name(),out_type));
    } 
    
    return def;
}

bool JZNodeFunction::compiler(JZNodeCompiler *c,QString &error)
{
    auto def = c->function(m_functionName);
    if (!def)
    {
        error = "函数不存在";
        return false;
    }

    JZFunctionDefine cur_def = functionDefine();
    if(!JZNodeType::functionTypeMatch(def,&cur_def))
    {
        error = "函数定义已改变,请更新," + def->delcare() + "," + cur_def.delcare();
        return false;
    }

    QList<int> in_list = pinInList(Pin_param);
    QList<int> out_list = pinOutList(Pin_param);
    Q_ASSERT(def->paramIn.size() == in_list.size() && def->paramOut.size() == out_list.size());

    bool input_ret = false;
    if(isFlowNode())
        input_ret = c->addFlowInput(m_id,error);
    else
        input_ret = c->addDataInput(m_id,error);
    if(!input_ret)        
        return false;
        
    QList<JZNodeIRParam> in,out;
    for(int i = 0; i < in_list.size(); i++)
        in << irId(c->paramId(m_id,in_list[i]));
    for(int i = 0; i < out_list.size(); i++)
        out << irId(c->paramId(m_id,out_list[i]));
    c->addCall(m_functionName,in,out);

    if (isFlowNode())
    {
        c->addFlowOutput(m_id);
        c->addJumpNode(flowOut());
    }
    return true;
}
