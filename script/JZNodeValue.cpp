#include "JZNodeValue.h"
#include "JZNodeCompiler.h"
#include "JZProject.h"
#include "JZNodeFactory.h"
#include "JZClassItem.h"
#include "JZNodeFunctionManager.h"

//JZNodeLiteral
JZNodeLiteral::JZNodeLiteral()
{
    m_type = Node_literal;
    addParamOut("out", Pin_dispValue | Pin_editValue);    
}

JZNodeLiteral::~JZNodeLiteral()
{
}

int JZNodeLiteral::dataType() const
{
    return pin(paramOut(0))->dataTypeId()[0];
}

void JZNodeLiteral::setDataType(int type)
{
    int out = paramOut(0);
    pin(out)->setDataTypeId({type});
    setName(JZNodeType::typeToName(type));
    if(JZNodeType::isBaseOrEnum(type))
        pin(out)->setFlag(Pin_out | Pin_param | Pin_dispValue | Pin_editValue);
    else
        pin(out)->setFlag(Pin_out | Pin_param | Pin_dispName);
    
    if (type == Type_bool)
        setLiteral("false");
    else if (type == Type_int)
        setLiteral("0");
    else if (type == Type_double)
        setLiteral("0.0");
    else if (type == Type_string)
        setLiteral("");    
    else if (type == Type_nullptr)
        setLiteral("null");
}

QString JZNodeLiteral::literal() const
{
    auto ptr = pin(paramOut(0));
    return ptr->value();
}

void JZNodeLiteral::setLiteral(const QString &value)
{
    setPinValue(paramOut(0), value);
}

bool JZNodeLiteral::compiler(JZNodeCompiler *c,QString &error)
{   
    int id = c->paramId(m_id,paramOut(0));    

    auto pin = this->pin(paramOut(0));
    QVariant value = JZNodeType::initValue(pin->dataTypeId()[0],pin->value());
    
    c->addSetVariable(irId(id),irLiteral(value));
    c->addNodeDebug(m_id);
    return true;
}

//JZNodeEnum
JZNodeEnum::JZNodeEnum()
{
    m_type = Node_enum;
    addParamOut("out", Pin_dispValue | Pin_editValue);    
}

JZNodeEnum::~JZNodeEnum()
{

}

bool JZNodeEnum::compiler(JZNodeCompiler *c, QString &error)
{
    QString key = pin(paramOut(0))->value();
    auto def = JZNodeObjectManager::instance()->enumMeta(m_name);
    int v = def->keyToValue(key);
    
    int id = c->paramId(m_id, paramOut(0));
    c->addSetVariable(irId(id), irLiteral(v));
    c->lastStatment()->memo = key;
    c->addNodeDebug(m_id);

    c->setPinType(m_id,paramOut(0),def->type());
    return true;
}

void JZNodeEnum::setEnum(QString type)
{
    auto meta = JZNodeObjectManager::instance()->enumMeta(type);    

    setName(meta->name());
    setPinType(paramOut(0), { meta->type() });
    setKey(meta->defaultKey());
}

void JZNodeEnum::setKey(QString key)
{
    setPinValue(paramOut(0), key);
}

void JZNodeEnum::setValue(int value)
{    
    auto meta = JZNodeObjectManager::instance()->enumMeta(m_name);
    setPinValue(paramOut(0), meta->valueToKey(value));
}

//JZNodeFlag
JZNodeFlag::JZNodeFlag()
{
    m_type = Node_flag;
    addParamOut("out", Pin_dispValue | Pin_editValue);
}

JZNodeFlag::~JZNodeFlag()
{
}

bool JZNodeFlag::compiler(JZNodeCompiler *c, QString &error)
{
    QString key = pin(paramOut(0))->value();
    auto def = JZNodeObjectManager::instance()->enumMeta(m_name);    
    int v = def->keyToValue(key);
    
    int id = c->paramId(m_id, paramOut(0));
    c->addSetVariable(irId(id), irLiteral(v));
    c->lastStatment()->memo = key;
    c->addNodeDebug(m_id);

    c->setPinType(m_id, paramOut(0), def->type());
    return true;
}

void JZNodeFlag::setFlag(QString type)
{
    auto meta = JZNodeObjectManager::instance()->enumMeta(type);

    setName(meta->name());
    setPinType(paramOut(0), { meta->type() });
    setKey(meta->defaultKey());
}

void JZNodeFlag::setKey(QString value)
{
    setPinValue(paramOut(0), value);
}

void JZNodeFlag::setValue(int value)
{
    auto def = JZNodeObjectManager::instance()->enumMeta(m_name);
    auto key = def->valueToKey(value);
    setPinValue(paramOut(0), key);
}

//JZNodeCreate
JZNodeCreate::JZNodeCreate()
{
    m_name = "createObject";
    m_type = Node_create;    

    int id = addParamIn("Class",Pin_editValue | Pin_dispName | Pin_dispValue | Pin_literal | Pin_noValue);
    setPinTypeString(id);
    addParamOut("Return", Pin_dispName);

    setPinTypeString(id);
}

JZNodeCreate::~JZNodeCreate()
{

}

void JZNodeCreate::setClassName(const QString &name)
{
    setPinValue(paramIn(0), JZNodeType::addQuote(name));
}

QString JZNodeCreate::className() const
{
    return JZNodeType::removeQuote(pinValue(paramIn(0)));    
}

bool JZNodeCreate::compiler(JZNodeCompiler *c,QString &error)
{
    if(className().isEmpty())
    {
        error = "没有设置类型";
        return false;
    }

    auto meta = JZNodeObjectManager::instance()->meta(className());    
    if (!meta)
    {
        error = "没有此类型:" + className();
        return false;
    }
    if(!c->addDataInput(m_id,error))
        return false;

    QString class_type = c->pinLiteral(m_id,paramIn(0)).toString();    
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam irIn = irLiteral(class_type);
    JZNodeIRParam irOut = irId(out_id);
    c->setPinType(m_id, paramOut(0), meta->id);

    int tmp_id = c->allocStack(Type_any);
    c->addCall("createObject", { irIn }, { irId(tmp_id) });
    c->addSetVariableConvert(irOut,irId(tmp_id));        
    return true;
}

void JZNodeCreate::onPinChanged(int id)
{
    if(id == paramIn(0))
    {
        int type = JZNodeObjectManager::instance()->getClassId(className());
        setPinType(paramOut(0),{type});
    }
}

//JZNodeCreateFromString
JZNodeCreateFromString::JZNodeCreateFromString()
{
    m_name = "createFromString";
    m_type = Node_createFromString;    

    int in1 = addParamIn("Class", Pin_editValue | Pin_dispName | Pin_dispValue | Pin_literal);
    int in2 = addParamIn("Context", Pin_editValue | Pin_dispName | Pin_dispValue );
    setPinTypeString(in1);
    setPinTypeString(in2);
    addParamOut("Return", Pin_dispName);
}

JZNodeCreateFromString::~JZNodeCreateFromString()
{

}

bool JZNodeCreateFromString::compiler(JZNodeCompiler *c, QString &error)
{
    QString class_name = className();
    if (class_name.isEmpty())
    {
        error = "没有设置类型";
        return false;
    }

    auto meta = JZNodeObjectManager::instance()->meta(class_name);
    if (!meta)
    {
        error = "没有此类型:" + class_name;
        return false;
    }
    auto func = meta->function("__fromString__");
    if(!func)
    {
        error = class_name + "未实现__fromString__";
        return false;
    }

    if (!c->addDataInput(m_id, error))
        return false;

    int out_id = c->paramId(m_id, paramOut(0));
    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(1)));
    out << irId(out_id);

    c->addCall(func, in, out);    
    return true;

}

void JZNodeCreateFromString::onPinChanged(int id)
{
    if (id == paramIn(0))
    {
        int type = JZNodeObjectManager::instance()->getClassId(className());
        setPinType(paramOut(0), { type });
    }
}

void JZNodeCreateFromString::setClassName(const QString &name)
{
    setPinValue(paramIn(0), JZNodeType::addQuote(name));
}

QString JZNodeCreateFromString::className() const
{
    return JZNodeType::removeQuote(pinValue(paramIn(0)));
}

void JZNodeCreateFromString::setContext(const QString &text)
{
    setPinValue(paramIn(1), text);
}

QString JZNodeCreateFromString::context() const
{
    return pinValue(paramIn(1));
}

//JZNodeFunctionPointer
JZNodeFunctionPointer::JZNodeFunctionPointer()
{
    m_type = Node_functionPointer;

    int out = addParamOut("",Pin_dispValue | Pin_editValue | Pin_literal);
    setPinType(out, {Type_function});
}

JZNodeFunctionPointer::~JZNodeFunctionPointer()
{

}

void JZNodeFunctionPointer::setFucntion(QString name)
{
    setParamOutValue(0, name);
}

QString JZNodeFunctionPointer::fucntion()
{
    return paramOutValue(0);
}

bool JZNodeFunctionPointer::compiler(JZNodeCompiler *c, QString &error)
{
    int id = c->paramId(m_id, paramOut(0));
    QString function_name = paramOutValue(0);

    auto func = JZNodeFunctionManager::instance()->function(function_name);
    if(!func)
    {
        error = "no such function " + function_name;
        return false;
    }

    JZFunctionPointer ptr;
    ptr.functionName = function_name;    
    c->addSetVariable(irId(id), irLiteral(QVariant::fromValue(ptr)));
    c->addNodeDebug(m_id);

    return true;
}

//JZNodeDisplay
JZNodeDisplay::JZNodeDisplay()
{
    m_type = Node_display;
    m_name = "display";

    int in = addParamIn("value",Pin_dispValue);
    setPinTypeAny(in);
}

JZNodeDisplay::~JZNodeDisplay()
{

}

bool JZNodeDisplay::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeDebug(m_id);
    return true;
}

//JZNodePrint
JZNodePrint::JZNodePrint()
{
    m_type = Node_print;
    m_name = "print";

    addFlowIn();
    addFlowOut();
    int in = addParamIn("text", Pin_dispName);
    setPinTypeAny(in);
}

JZNodePrint::~JZNodePrint()
{
}

bool JZNodePrint::compiler(JZNodeCompiler *c,QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    auto in_id = irId(c->paramId(m_id, paramIn(0)));
    bool need_format = false;

    if(need_format)
    {
        QList<JZNodeIRParam> fmt_in, fmt_out;
        auto in_list = paramInList();
        for(int i = 0; i < in_list.size(); i++)
        {
            fmt_in << irId(c->paramId(m_id,in_list[i]));
        }
        c->addCallConvert("format",fmt_in,fmt_out);
    }

    QList<JZNodeIRParam> in, out;
    in << in_id;
    c->addCall("print", in, out);    
    c->addJumpNode(flowOut());

    return true;
}

//JZNodeThis
JZNodeThis::JZNodeThis()
{
    m_name = "this";
    m_type = Node_this;
    addParamOut("this",Pin_dispName);
}

JZNodeThis::~JZNodeThis()
{

}

bool JZNodeThis::compiler(JZNodeCompiler *c,QString &error)
{
    int out_id = c->paramId(m_id, paramOut(0));
    c->addNodeDebug(m_id);    
    c->addSetVariable(irId(out_id),irThis());    
    return true;
}

bool JZNodeThis::update(QString &error) 
{
    auto class_file = m_file->project()->getItemClass(m_file);
    if(!class_file){
        error = "this not define";
        return false;
    }

    QString className = class_file->className();
    int data_type = JZNodeObjectManager::instance()->getClassId(className);
    Q_ASSERT(data_type != Type_none);
    setPinType(paramOut(0),{data_type});
    return false;
}

//JZNodeParam
JZNodeParam::JZNodeParam()
{
    m_name = "get";
    m_flag = NodeProp_dragVariable;
    m_type = Node_param;
    
    addParamOut("", Pin_dispName);    
}

JZNodeParam::~JZNodeParam()
{
}

bool JZNodeParam::compiler(JZNodeCompiler *c,QString &error)
{    
    QString name = variable();    
    if (!c->checkVariableExist(name, error))
        return false;
    auto def = c->getVariableInfo(name);
    
    c->addNodeDebug(m_id);
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam ref = c->paramRef(name);    
    c->setPinType(m_id, paramOut(0), def->dataType());
    c->addSetVariable(irId(out_id), ref);
    return true;
}

void JZNodeParam::setVariable(const QString &name)
{
    setPinName(paramOut(0), name);
}

QString JZNodeParam::variable() const
{
    return pinName(paramOut(0));
}

void JZNodeParam::drag(const QVariant &value)
{
    setVariable(value.toString());
}

void JZNodeParam::onPinChanged(int id)
{
    if(id == paramOut(0))
    {
        auto def = JZNodeCompiler::getVariableInfo(m_file, variable());        
        int dataType = def? def->dataType() : Type_none;        
        if (dataType != Type_none)
            setPinType(id, { dataType });
        else
            clearPinType(id);
    }
}

//JZNodeSetParam
JZNodeSetParam::JZNodeSetParam()
{
    m_type = Node_setParam;
    m_flag = NodeProp_dragVariable;
    m_name = "set";

    addFlowIn();    
    addFlowOut();

    int in = addParamIn("name", Pin_dispName | Pin_editValue | Pin_dispValue | Pin_literal | Pin_noValue);
    setPinTypeString(in);

    addParamIn("value", Pin_dispName | Pin_editValue | Pin_dispValue);
    addParamOut("");
}

JZNodeSetParam::~JZNodeSetParam()
{
}

void JZNodeSetParam::setVariable(const QString &name)
{
    setPinValue(paramIn(0), JZNodeType::addQuote(name));
}

QString JZNodeSetParam::variable() const
{
    return JZNodeType::removeQuote(pinValue(paramIn(0)));
}

void JZNodeSetParam::setValue(const QString &name)
{
    setPinValue(paramIn(1), name);
}

QString JZNodeSetParam::value() const
{
    return pinValue(paramIn(1));
}

void JZNodeSetParam::drag(const QVariant &value)
{
    setVariable(value.toString());
}

void JZNodeSetParam::onPinChanged(int id)
{
    if(id == paramIn(0))
    {
        auto def = JZNodeCompiler::getVariableInfo(m_file, variable());        
        int dataType = def? def->dataType() : Type_none;
        if (dataType != Type_none)
        {
            setPinType(paramIn(1), { dataType });
            setPinType(paramOut(0), { dataType });
        }
        else
        {
            clearPinType(paramIn(1));
            clearPinType(paramOut(0));
        }
    }
}

bool JZNodeSetParam::compiler(JZNodeCompiler *c,QString &error)
{
    QString name = variable();
    if (!c->checkVariableExist(name, error))
        return false;
    if(!c->addFlowInput(m_id,error))
        return false;
        
    int id = c->paramId(m_id,paramIn(1));
    int m_out = c->paramId(m_id,paramOut(0));
    
    JZNodeIRParam ref = c->paramRef(name);
    c->addSetVariable(ref,irId(id));
    c->addSetVariable(irId(m_out),irId(id));    
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeSetParamDataFlow
JZNodeSetParamDataFlow::JZNodeSetParamDataFlow()
{
    m_name = "set";
    m_flag = NodeProp_dragVariable;
    m_type = Node_setParamData;

    int in = addParamIn("name",Pin_dispName | Pin_editValue | Pin_dispValue | Pin_literal | Pin_noValue);
    setPinTypeString(in);

    addParamIn("value",Pin_dispName | Pin_editValue | Pin_dispValue);
}

JZNodeSetParamDataFlow::~JZNodeSetParamDataFlow()
{
}

void JZNodeSetParamDataFlow::setVariable(const QString &name)
{
    setPinValue(paramIn(0),JZNodeType::addQuote(name));
}

QString JZNodeSetParamDataFlow::variable() const
{
    return JZNodeType::removeQuote(pinValue(paramIn(0)));
}

void JZNodeSetParamDataFlow::setValue(const QString &name)
{
    setPinValue(paramIn(1), name);
}

QString JZNodeSetParamDataFlow::value() const
{
    return pinValue(paramIn(1));
}

void JZNodeSetParamDataFlow::drag(const QVariant &value)
{
    setVariable(value.toString());
}

void JZNodeSetParamDataFlow::onPinChanged(int id)
{
    if(id == paramIn(0))
    {
        auto def = JZNodeCompiler::getVariableInfo(m_file,variable());
        int dataType = def? def->dataType() : Type_none;        
        if (dataType != Type_none)        
            setPinType(paramIn(1), { dataType });                    
        else        
            clearPinType(paramIn(1));
    }
}

bool JZNodeSetParamDataFlow::compiler(JZNodeCompiler *c,QString &error)
{
    QString name = variable();
    if (!c->checkVariableExist(name, error))
        return false;
    if(!c->addDataInput(m_id,error))
        return false;
    
    int id = c->paramId(m_id,paramIn(1));
    c->addSetVariable(c->paramRef(name),irId(id));
    return true;
}

//JZNodeAbstractMember
JZNodeAbstractMember::JZNodeAbstractMember()
{
    addParamIn("Object",Pin_dispName);
    setPinType(paramIn(0), { Type_object });
}

JZNodeAbstractMember::~JZNodeAbstractMember()
{

}

void JZNodeAbstractMember::setMember(QStringList param_list)
{
    auto pin_list = memberPinList();
    for (int i = 0; i < pin_list.size(); i++)
    {
        if (!param_list.contains(pinName(pin_list[i])))
            removePin(pin_list[i]);
    }

    auto cur_member = member();
    for (int i = 0; i < param_list.size(); i++)
    {        
        if (cur_member.contains(param_list[i]))
        {
            int flag = Pin_dispName;
            if (m_type == Node_setMemberParam || m_type == Node_setMemberParamData)
                addParamIn(param_list[i], flag);
            else
                addParamOut(param_list[i], flag);
        }
    }

    std::sort(m_pinList.begin(), m_pinList.end(), [this](const JZNodePin &pin1, const JZNodePin &pin2) {
        if (m_type == Node_setMemberParam || m_type == Node_setMemberParamData)
        {
            if (pin1.id() == 0)
                return true;
            if (pin2.id() == 0)
                return false;
        }
        return pin1.name() < pin2.name();
    });

    updateMemberType();
}

QString JZNodeAbstractMember::className()
{
    return pin(paramIn(0))->name();
}

QStringList JZNodeAbstractMember::member()
{
    QStringList ret;
    QList<int> pin_list = memberPinList();    
    for (int i = 0; i < pin_list.size(); i++)
        ret << pin(pin_list[i])->name();
    return ret;
}

QList<int> JZNodeAbstractMember::memberPinList()
{
    QList<int> pin_list;
    if (m_type == Node_setMemberParam || m_type == Node_setMemberParamData)
    {
        pin_list = paramInList();
        pin_list.removeAt(0);
    }
    else
        pin_list = paramOutList();

    return pin_list;
}

void JZNodeAbstractMember::onPinLinked(int id)
{
    if (id == paramIn(0))
        updateMemberType();
}

void JZNodeAbstractMember::updateMemberType()
{
    auto pin_id = m_file->getConnectPin(m_id, paramIn(0));
    if (pin_id.size() == 0)
        return;

    auto gemo = m_file->getConnect(pin_id[0]);
    auto type = m_file->getNode(gemo->from.nodeId)->pinTypeInt(gemo->from.pinId);
    if (type.size() == 1)
        updateMemberType(type[0]);
}

void JZNodeAbstractMember::updateMemberType(int type)
{
    auto meta = JZNodeObjectManager::instance()->meta(type);
    if (!meta)
        return;    

    QList<int> pin_list = memberPinList();
    for (int i = 0; i < pin_list.size(); i++)
    {
        auto name = pinName(pin_list[i]);
        auto param = meta->param(name);
        if (param)
            setPinType(pin_list[i], { param->dataType() });
        else
            setPinType(pin_list[i], {});
    }
}

//JZNodeMemberParam
JZNodeMemberParam::JZNodeMemberParam()
{
    m_name = "getMember";
    m_type = Node_memberParam;

    addParamIn("", Pin_dispName);
}

JZNodeMemberParam::~JZNodeMemberParam()
{

}


bool JZNodeMemberParam::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addDataInput(m_id, error))
        return false;

    int obj_id = c->paramId(m_id, paramIn(0));
    auto list = paramOutList();
    for (int i = 0; i < list.size(); i++)
    {
        QList<JZNodeIRParam> in,out;
        in << irId(obj_id);
        in << irLiteral(pin(list[i])->name());
        out << irId(c->paramId(m_id, list[i]));
        c->addCall("getMemberParam", in, out);
    }
    return true;
}

//JZNodeSetMemberParam
JZNodeSetMemberParam::JZNodeSetMemberParam()
{
    m_name = "setMember";
    m_type = Node_setMemberParam;

    addParamIn("", Pin_dispName);
    addFlowIn();
    addFlowOut();    
}

JZNodeSetMemberParam::~JZNodeSetMemberParam()
{

}


bool JZNodeSetMemberParam::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id,error))
        return false;

    int obj_id = c->paramId(m_id, paramIn(0));        
    QList<JZNodeIRParam> in, out;
    in << irId(obj_id);
    in << irId(0);
    in << irId(0);

    auto list = paramInList();
    for (int i = 0; i < list.size(); i++)
    {
        int var_id = c->paramId(m_id, paramIn(i + 1));
        QString var_name = pin(paramIn(i + 1))->name();
        in[1] = irId(var_id);
        in[2] = irLiteral(var_name);
        c->addCall("setMemberParam", in, out);    
    }

    c->addJumpNode(flowOut());
    return true;
}

//JZNodeSetMemberData
JZNodeSetMemberParamData::JZNodeSetMemberParamData()
{
    m_name = "setMember";
    m_type = Node_setMemberParamData;

    addParamIn("",Pin_dispName);    
}

JZNodeSetMemberParamData::~JZNodeSetMemberParamData()
{

}

bool JZNodeSetMemberParamData::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addDataInput(m_id, error))
        return false;

    int obj_id = c->paramId(m_id, paramIn(0));
    int var_id = c->paramId(m_id, paramIn(1));

    QList<JZNodeIRParam> in, out;
    in << irId(obj_id);
    in << irLiteral(pin(paramIn(1))->name());
    in << irId(var_id);
    c->addCall("setMemberParam", in, out);

    c->addJumpNode(flowOut());
    return true;
}

//JZNodeClone
JZNodeClone::JZNodeClone()
{
    m_name = "clone";
    m_type = Node_clone;

    auto in = addParamIn("");
    auto out = addParamOut("");
    setPinTypeAny(in);
    setPinTypeAny(out);
}

JZNodeClone::~JZNodeClone()
{

}

bool JZNodeClone::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addDataInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in;
    QList<JZNodeIRParam> out;
    in << irId(c->paramId(m_id, paramIn(0)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall("clone", in, out);
    return true;
}

//JZNodeSwap
JZNodeSwap::JZNodeSwap()
{
    m_name = "swap";
    m_type = Node_swap;

    addFlowIn();
    addFlowOut();

    int in1 = addParamIn("");
    int in2 = addParamIn("");
    setPinTypeAny(in1);
    setPinTypeAny(in2);
}

JZNodeSwap::~JZNodeSwap()
{

}

bool JZNodeSwap::compiler(JZNodeCompiler *c, QString &error)
{    
    enum {
        swap_none,
        swap_param,
        swap_list,
        swap_map,
    };

    auto getType = [](JZNode *node)->int{
        if (node->type() == Node_param)
            return swap_param;
        else if (node->type() == Node_function)
        {
            if (node->name() == "List.get")
                return swap_list;
            else if (node->name() == "Map.get")
                return swap_param;
        }
        return swap_none;
    }; 

    auto setType = [c](int type,JZNode *node,int id)->int {
        if (node->type() == Node_param)
            return swap_param;
        else if (node->type() == Node_function)
        {            
            if (type == swap_param)
            {
                int param_id = c->paramId(node->id(), node->paramIn(0));
                c->addSetVariable(irId(param_id), irId(id));
            }
            else
            {
                QList<JZNodeIRParam> in;
                QList<JZNodeIRParam> out;
                in << irId(c->paramId(node->id(), node->paramIn(0)));
                in << irId(c->paramId(node->id(), node->paramIn(1)));
                in << irId(id);
                if (type == swap_list)
                {
                    c->addCall("List.set", in, out);
                }
                else if (type == swap_map)
                {
                    c->addCall("Map.set", in, out);
                }
            }
        }
        return swap_none;
    };

    if (!c->addFlowInput(m_id, error))
        return false;

    auto graph = c->currentGraph();
    GraphNode *node = graph->graphNode(m_id);
    auto in1 = paramIn(0);
    auto in2 = paramIn(1);   
    auto &in1_list = node->paramIn[in1];
    auto &in2_list = node->paramIn[in2];    
    auto in1_node = graph->node(in1_list[0].nodeId);
    auto in2_node = graph->node(in2_list[0].nodeId);

    int in1_type = getType(in1_node);
    int in2_type = getType(in2_node);
    if (in1_type == swap_none || in2_type == swap_none)
    {
        error = "不支持此类转换";
        return false;
    }

    auto in1_id = c->paramId(m_id, in1);
    auto in2_id = c->paramId(m_id, in2);
    setType(in1_type, in1_node, in2_id);
    setType(in2_type, in2_node, in1_id);

    c->addJumpNode(flowOut());

    return true;
}