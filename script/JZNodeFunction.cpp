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

bool JZNodeFunction::isMemberCall()
{
    if (!m_file)
        return false;
    auto class_item = m_file->getClassFile();
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
        pin.setFlag(Pin_param | Pin_in | Pin_dispName);
        pin.setDataType({define->paramIn[i].type });        
        pin.setValue(define->paramIn[i].value);
        addPin(pin);
    }

    for(int i = 0; i < define->paramOut.size(); i++)
    {
        JZNodePin pin;
        pin.setName(define->paramOut[i].name);
        pin.setFlag(Pin_param | Pin_out | Pin_dispName);
        pin.setDataType({define->paramOut[i].type});
        addPin(pin);
    }

    if(define->isMemberFunction())
    {
        auto pin = this->pin(paramIn(0));
        pin->setFlag(pin->flag() | Pin_editValue);
        pin->setEditType(Type_paramName);
        m_flag &= NodeProp_dragVariable; 
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

bool JZNodeFunction::update(QString &error)
{
    auto env = environment();
    auto func = env->functionManager()->function(m_functionName);
    if (!func)
    {
        error = "函数不存在";
        return false;
    }

    JZFunctionDefine cur_def = functionDefine();
    if (!JZNodeType::functionTypeMatch(func, &cur_def))
    {
        error = "函数定义已改变,请更新," + func->delcare() + "," + cur_def.delcare();
        return false;
    }

    updateName();       
    if (func)
    {
        for (int i = 0; i < func->paramIn.size(); i++)
        {
            auto pin = this->pin(paramIn(i));
            if (JZNodeType::isBaseOrEnum(env->nameToType(func->paramIn[i].type)))
                pin->changeFlag(Pin_dispValue | Pin_editValue, true);
            else
                pin->changeFlag(Pin_dispValue | Pin_editValue, false);
        }
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
        c->setPinType(m_id, paramIn(0), Type_ignore);

    bool input_ret = false;
    if(isFlowNode())
        input_ret = c->addFlowInput(m_id,error);
    else
        input_ret = c->addDataInput(m_id,error);
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

        int this_id = JZNodeGemo::paramId(m_id, paramIn(0));
        c->addSetVariable(irId(this_id), irRef(name));
    }
        
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