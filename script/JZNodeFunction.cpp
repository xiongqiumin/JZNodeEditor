#include "JZNodeFunction.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"

//JZNodeFunction
JZNodeFunction::JZNodeFunction()
{
    m_type = Node_function;
    m_directCall = true;
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

void JZNodeFunction::updateName()
{
    if (!m_file)
        return;

    auto func_inst = environment()->functionManager();
    setName(m_functionName);
    auto meta = func_inst->function(m_functionName);
    if(meta && meta->isMemberFunction())
    {
        QString v = variable();
        if (v.isEmpty())
        {
            if (!isMemberCall())
                return;

            v = "this";
        }

        QString name = v + "." + meta->name;
        setName(name);       
    }
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
    updateName();
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

void JZNodeFunction::setFunction(const JZFunctionDefine *define)
{
    Q_ASSERT(define);            

    auto env = environment();
    m_functionName = define->fullName();
    
    clearPin();
    if(define->isFlowFunction)
    {
        addFlowIn();
        addFlowOut();
    }

    for(int i = 0; i < define->paramIn.size(); i++)
    {
        JZNodePin pin;
        pin.setName(define->paramIn[i].name);    
        pin.setFlag(Pin_param | Pin_in);
        pin.setDataType({define->paramIn[i].type });        
        pin.setValue(define->paramIn[i].value);
        addPin(pin);
    }

    for(int i = 0; i < define->paramOut.size(); i++)
    {
        JZNodePin pin;
        pin.setName(define->paramOut[i].name);
        pin.setFlag(Pin_param | Pin_out);
        pin.setDataType({define->paramOut[i].type});
        addPin(pin);
    }

    if(define->isMemberFunction())
    {
        auto pin = this->pin(paramIn(0));
        pin->setFlag(pin->flag());
    }

    setName(define->fullName());
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
        QString in_type = in->dataType()[0];
        def.paramIn.push_back(JZParamDefine(in->name(),in_type));
    }

    auto out_list = paramOutList();
    for(int i = 0; i < out_list.size(); i++)
    {
        auto out = pin(out_list[i]);
        QString out_type = out->dataType()[0];
        def.paramOut.push_back(JZParamDefine(out->name(),out_type));
    } 
    
    return def;
}

bool JZNodeFunction::updateNode(QString &error)
{
    auto env = environment();
    auto func = env->functionManager()->function(m_functionName);
    if (!func)
    {
        error = "函数不存在";
        return false;
    }

    JZFunctionDefine cur_def = functionDefine();
    if (!env->isFunctionTypeMatch(func, &cur_def))
    {
        error = "函数定义已改变,请更新," + func->delcare() + "," + cur_def.delcare();
        return false;
    }

    updateName();
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