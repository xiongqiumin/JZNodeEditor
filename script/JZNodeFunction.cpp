#include "JZNodeFunction.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"

//JZNodeFunction
JZNodeFunction::JZNodeFunction()
{
    m_type = Node_function;
    m_name = "function";
    m_directCall = false;
}

JZNodeFunction::~JZNodeFunction()
{

}

bool JZNodeFunction::isMemberCall()
{
    Q_ASSERT(m_file);
    auto class_item = m_file->getClassItem();
    if (!class_item)
        return false;

    auto env = environment();
    auto meta = env->functionManager()->function(m_functionName);
    if (meta && meta->isMemberFunction() && env->isInherits(class_item->className(),meta->className))
        return true;

    return false;
}

void JZNodeFunction::setDirectCall(bool flag)
{
    m_directCall = flag;
}

bool JZNodeFunction::isDirectCall()
{
    return m_directCall;
}

void JZNodeFunction::setVariable(const QString &name)
{
    setPinValue(paramIn(0),name);    
}

QString JZNodeFunction::variable() const
{
    return pinValue(paramIn(0));
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

void JZNodeFunction::setFunction(QString fullName)
{
    m_functionName = fullName;
    update();
}

void JZNodeFunction::setFunction(const JZFunctionDefine *define)
{    
    m_functionName = define->fullName();
    QString error;
    updateFunctionDefine(define,error);
}
    

QString JZNodeFunction::function() const
{
    return m_functionName;
}

bool JZNodeFunction::updateNode(QString &error)
{
    auto env = environment();
    auto define = env->functionManager()->function(m_functionName);
    if (!define)
    {
        error = "函数不存在";
        return false;
    }

    if (!updateFunctionDefine(define, error))
        return false;
                
    return true;
}

bool JZNodeFunction::updateFunctionDefine(const JZFunctionDefine *define,QString &error)
{
    if (define->isFlowFunction)
    {
        if (flowInCount() == 0)
            addFlowIn();
        if (flowOutCount() == 0)
            addFlowOut();
    }

    if (paramInCount() < define->paramIn.size())
    {
        int count = paramInCount();
        for (int i = count; i < define->paramIn.size(); i++)
        {
            addParamIn("");
        }
    }

    if (paramOutCount() < define->paramOut.size())
    {
        int count = paramOutCount();
        for (int i = count; i < define->paramOut.size(); i++)
        {
            addParamOut("");
        }
    }

    auto pin_in_list = paramInList();
    auto pin_out_list = paramOutList();
    for (int i = 0; i < define->paramIn.size(); i++)
    {
        auto pin = this->pin(pin_in_list[i]);
        pin->setName(define->paramIn[i].name);
        pin->setDataType({ define->paramIn[i].type });
    }

    for (int i = 0; i < define->paramOut.size(); i++)
    {
        auto pin = this->pin(pin_out_list[i]);
        pin->setName(define->paramOut[i].name);
        pin->setDataType({ define->paramOut[i].type });
    }

    if (paramInCount() != define->paramIn.size())
    {
        error = "函数没有" + QString::number(paramInCount()) + "输入";
        return false;
    }
    if (paramOutCount() != define->paramOut.size())
    {
        error = "函数不存在" + QString::number(paramOutCount()) + "输出";
        return false;
    }

    return true;
}

bool JZNodeFunction::compiler(JZNodeCompiler *c,QString &error)
{
    auto env = environment();
    auto def = c->function(m_functionName);       
    
    QList<int> in_list = pinInList(Pin_param);
    QList<int> out_list = pinOutList(Pin_param);
    Q_ASSERT(def->paramIn.size() == in_list.size() && def->paramOut.size() == out_list.size());
    
    if (def->isMemberFunction() && c->isPinLiteral(m_id, paramIn(0)))
        in_list.removeAt(0);

    bool input_ret = false;
    if(isFlowNode())
        input_ret = c->addFlowInput(m_id, in_list,error);
    else
        input_ret = c->addDataInput(m_id, in_list,error);
    if(!input_ret)        
        return false;

    if (def->isMemberFunction() && c->isPinLiteral(m_id, paramIn(0)))
    {
        int this_type = env->nameToType(def->paramIn[0].type);
        c->setPinType(m_id, paramIn(0), this_type);
        QString name = c->pinLiteral(m_id, paramIn(0));
        if (name.isEmpty())
        {
            if (!isMemberCall())
            {
                error = "input1未设置";
                return false;
            }
            name = "this";
        }        
        if (!c->checkVariableType(name, this_type, error))
            return false;

        int this_id = c->paramId(m_id, paramIn(0));
        c->addSetVariableConvert(irId(this_id), irRef(name));

        in_list.insert(0, paramIn(0));
    }
        
    QList<JZNodeIRParam> in,out;
    for(int i = 0; i < in_list.size(); i++)
        in << irId(c->paramId(m_id,in_list[i]));
    for(int i = 0; i < out_list.size(); i++)
        out << irId(c->paramId(m_id,out_list[i]));
    
    if(m_directCall || !def->isVirtualFunction)
        c->addCall(m_functionName,in,out);
    else
        c->addCallVirtual(m_functionName,in,out);

    if (isFlowNode())
    {
        c->addFlowOutput(m_id);
    }
    return true;
}

//JZNodeFunctionCustom
JZNodeFunctionCustom::JZNodeFunctionCustom()
{
}

JZNodeFunctionCustom::~JZNodeFunctionCustom()
{
}

void JZNodeFunctionCustom::setFunction(const QString &name)
{
    m_functionName = name;
    initFunction();
}

QString JZNodeFunctionCustom::function() const
{
    return m_functionName;
}