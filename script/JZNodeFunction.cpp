#include "JZNodeFunction.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"
#include "JZContainer.h"
#include "JZScriptItemVisitor.h"

//JZNodeFunction
JZNodeFunction::JZNodeFunction()
{
    m_type = Node_function;
    m_name = "function";
    m_directCall = false;
    m_forceFlow = false;
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

void JZNodeFunction::setForceFlow(bool forceFlow)
{
    m_forceFlow = forceFlow;
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
    s << m_functionName << m_directCall << m_forceFlow;
}

void JZNodeFunction::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
    s >> m_functionName >> m_directCall >> m_forceFlow;
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
    if (m_functionName.isEmpty())
    {
        error = "函数名为空";
        return false;
    }

    auto define = JZNodeCompiler::function(file(),m_functionName);
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
    if (define->isFlowFunction || m_forceFlow)
    {
        if (flowInCount() == 0)
            addFlowIn();
        if (flowOutCount() == 0)
            addFlowOut();
    }
    else
    {
        if (flowInCount() == 1)
            removePin(flowIn());
        if (flowOutCount() == 1)
            removePin(flowOut(0));
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

//JZNodeContainerFunction
JZNodeContainerFunction::JZNodeContainerFunction()
{
    m_type = Node_genericFunction;
    m_name = "function";
    m_forceFlow = false;
}

JZNodeContainerFunction::~JZNodeContainerFunction()
{
}

void JZNodeContainerFunction::setForceFlow(bool forceFlow)
{
    m_forceFlow = forceFlow;
    updateFunction();
}

void JZNodeContainerFunction::setFunction(QString fullName)
{
    m_functionName = fullName;
    updateFunction();
    update();
}

QString JZNodeContainerFunction::function() const
{
    return m_functionName;
}

void JZNodeContainerFunction::setVariable(const QString& name)
{    
    setPinValue(paramIn(0), name);
}

QString JZNodeContainerFunction::variable() const
{
    return pinValue(paramIn(0));
}

QString JZNodeContainerFunction::valueType()
{
    return m_valueType;
}

void JZNodeContainerFunction::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
    s << m_functionName << m_forceFlow;
}

void JZNodeContainerFunction::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
    s >> m_functionName >> m_forceFlow;
    updateFunction();
}

void JZNodeContainerFunction::updateFunction()
{    
    clearPin();
    
    if(m_functionName.isEmpty())
        return;

    JZFunctionDefine *func = JZContainerManager::instance()->function(m_functionName);
    if (func->isFlowFunction || m_forceFlow)
    {
        addFlowIn();
        addFlowOut();
    }

    for (int i = 0; i < func->paramIn.size(); i++)
    {
        int in = addParamIn(func->paramIn[i].name);
        setPinType(in, { func->paramIn[i].type });
    }

    for (int i = 0; i < func->paramOut.size(); i++)
    {
        int out = addParamOut(func->paramOut[i].name);
        setPinType(out, { func->paramOut[i].type });
    }
}

QString JZNodeContainerFunction::realFunctionName()
{
    QString functionName = m_functionName;
    if(m_functionName.startsWith("QList<"))
        functionName.replace("T", m_valueType);
    else if(m_functionName.startsWith("QMap<"))
    {
        functionName.replace("K", m_keyType);
        functionName.replace("V", m_valueType);
    }   
    return functionName;
}

bool JZNodeContainerFunction::updateNode(QString &error)
{
    auto contianer_manager = JZContainerManager::instance();
    auto env = m_file->project()->environment();
    JZFunctionDefine *func = contianer_manager->function(m_functionName);    
    
    auto input_pin = m_file->getConnectInput(m_id, paramIn(0));
    if (input_pin.size() == 0)
    {
        error = "无法确定泛形函数类型";
        return false;
    }

    QStringList input_class_type;
    for (int i = 0; i < input_pin.size(); i++)
    {
        auto pin_gemo = m_file->getConnect(input_pin[i])->from;
        auto pin = m_file->getPin(pin_gemo);
        input_class_type << pin->dataType();
    }
    input_class_type = input_class_type.toSet().values();

    QString class_type = env->upType(input_class_type);
    if (env->nameToType(class_type) == Type_none)
    {
        error = "无法确定泛形函数类型";
        return false;
    }

    QString gen_class = JZFunctionHelper::splitFunction(m_functionName).className;
    GenericInfo base_class_gen = contianer_manager->genericInfo(gen_class);
    GenericInfo class_gen = contianer_manager->genericInfo(class_type);
    if(base_class_gen.className != class_gen.className)
    {
        error = "需要传入" + gen_class;
        return false;
    }

    m_valueType = class_gen.generics[0];
    func->paramIn[0].type = class_type + "*";
    for (int i = 1; i < func->paramIn.size(); i++)
    {
        if (func->paramIn[i].type == "T")
        {
            if (m_valueType.isEmpty())
                setPinType(paramIn(i), QStringList());
            else
                setPinType(paramIn(i), { m_valueType });
        }
    }

    for (int i = 0; i < func->paramOut.size(); i++)
    {
        if (func->paramOut[i].type == "T")
        {
            if (m_valueType.isEmpty())
                setPinType(paramOut(i), QStringList());
            else
                setPinType(paramOut(i), { m_valueType });
        }
    }

    return m_valueType.isEmpty();
}

bool JZNodeContainerFunction::compiler(JZNodeCompiler *c, QString &error)
{
    Q_ASSERT(c->env()->isVaildType(m_valueType));

    QString functionName = realFunctionName();

    QList<int> in_list = paramInList();
    QList<int> out_list = paramOutList();

    bool input_ret = false;
    if (isFlowNode())
        input_ret = c->addFlowInput(m_id, in_list, error);
    else
        input_ret = c->addDataInput(m_id, in_list, error);
    if (!input_ret)
        return false;

    QList<JZNodeIRParam> in, out;
    for (int i = 0; i < in_list.size(); i++)
        in << irId(c->paramId(m_id, in_list[i]));
    for (int i = 0; i < out_list.size(); i++)
        out << irId(c->paramId(m_id, out_list[i]));

    c->addCall(functionName, in, out);

    if (isFlowNode())
        c->addFlowOutput(m_id);

    return true;
}