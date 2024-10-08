﻿#include <QPushButton>
#include <QHBoxLayout>
#include <QLineEdit>
#include "JZNodeValue.h"
#include "JZNodeCompiler.h"
#include "JZProject.h"
#include "JZNodeFactory.h"
#include "JZClassItem.h"
#include "JZNodeFunctionManager.h"
#include "JZNodePinWidget.h"
#include "JZNodeFunction.h"
#include "JZNodeUtils.h"

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
    auto env = environment();
    return env->nameToType(pin(paramOut(0))->dataType()[0]);
}

void JZNodeLiteral::setDataType(int type)
{    
    Q_ASSERT(JZNodeType::isBase(type) || type == Type_nullptr);

    int out = paramOut(0);        
    pin(out)->setDataType({ JZNodeType::typeName(type) });
    setName(JZNodeType::typeName(type));
    pin(out)->setFlag(Pin_out | Pin_param | Pin_dispValue | Pin_editValue);        
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
    auto env = environment();
    int id = c->paramId(m_id,paramOut(0));    

    auto pin = this->pin(paramOut(0));
    int data_type = env->nameToType(pin->dataType()[0]);
    QVariant value = env->initValue(data_type,pin->value());
    
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
    auto obj_inst = environment()->objectManager();
    auto def = obj_inst->enumMeta(m_name);
    int v = def->keyToValue(key);
    
    int id = c->paramId(m_id, paramOut(0));
    c->addSetVariable(irId(id), irLiteral(v));
    c->lastStatment()->memo = key;
    c->addNodeDebug(m_id);

    c->setPinType(m_id,paramOut(0),def->id());
    return true;
}

void JZNodeEnum::setEnum(const JZNodeEnumDefine *meta)
{    
    setName(meta->name());
    setPinType(paramOut(0), { meta->name() });
    setKey(meta->defaultKey());
}

void JZNodeEnum::setKey(QString key)
{
    setPinValue(paramOut(0), key);
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
    auto obj_inst = environment()->objectManager();
    auto def = obj_inst->enumMeta(m_name);    
    int v = def->keyToValue(key);
    
    int id = c->paramId(m_id, paramOut(0));
    c->addSetVariable(irId(id), irLiteral(v));
    c->lastStatment()->memo = key;
    c->addNodeDebug(m_id);

    c->setPinType(m_id, paramOut(0), def->id());
    return true;
}

void JZNodeFlag::setFlag(const JZNodeEnumDefine *meta)
{
    setName(meta->name());
    setPinType(paramOut(0), { meta->name() });
    setKey(meta->defaultKey());
}

void JZNodeFlag::setKey(QString value)
{
    setPinValue(paramOut(0), value);
}

//JZNodeConvert
JZNodeConvert::JZNodeConvert()
{
    m_name = "convert";
    m_type = Node_convert;   

    int in = addParamIn("var", Pin_param | Pin_dispName);
    setPinTypeArg(in);
    addParamOut("", Pin_param | Pin_widget);
}

JZNodeConvert::~JZNodeConvert()
{
}

void JZNodeConvert::setOutputType(int type)
{
    auto env = environment();
    QString name = env->typeToName(type);
    setPinValue(paramOut(0), name);
    
    QString error;
    update(error);
}

bool JZNodeConvert::compiler(JZNodeCompiler *c, QString &error)
{    
    if(!c->addDataInput(m_id,error))
        return false;

    auto env = environment();
    int in_type = c->pinType(m_id,paramIn(0));
    QString out_type_str = pinType(paramOut(0))[0];
    int out_type = env->nameToType(out_type_str);
    if(!env->canConvertExplicitly(in_type, out_type))
    {   
        error = "无法将类型" + env->typeToName(in_type) + "转换到" + out_type_str;
        return false;
    }

    int in_id = c->paramId(m_id,paramIn(0));
    int out_id = c->paramId(m_id,paramOut(0));
    c->addConvert(irId(in_id),out_type,irId(out_id));
    return true;
}

JZNodePinWidget* JZNodeConvert::createWidget(int id)
{
    auto w = new JZNodePinValueWidget(this, id);
    w->initWidget(Type_string);
    return w;
}

bool JZNodeConvert::update(QString &error)
{
    auto env = environment();
    int id = paramOut(0);
    QString name = pinValue(id);
    setName("convert to " + name + "");
    int type = env->nameToType(name);
        
    if (type != Type_none)
    {
        setPinType(paramOut(0), { name });
        return true;
    }
    else
    {
        error = JZNodeCompiler::errorString(Error_noClass,{ name });
        clearPinType(paramOut(0));
        return false;
    }
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
    setPinValue(paramIn(0), name);
}

QString JZNodeCreate::className() const
{
    return pinValue(paramIn(0));    
}

bool JZNodeCreate::compiler(JZNodeCompiler *c,QString &error)
{
    if(className().isEmpty())
    {
        error = "没有设置类型";
        return false;
    }

    auto obj_inst = environment()->objectManager();
    auto meta = obj_inst->meta(className());    
    if (!meta)
    {
        error = "没有此类型:" + className();
        return false;
    }
    if(!c->addDataInput(m_id,error))
        return false;

    QString class_type = c->pinLiteral(m_id,paramIn(0));    
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam irIn = irLiteral(class_type);
    JZNodeIRParam irOut = irId(out_id);
    c->setPinType(m_id, paramOut(0), meta->id);

    c->addCall("createObject", { irIn }, { irOut });
    return true;
}

bool JZNodeCreate::update(QString &error)
{    
    auto obj_inst = environment()->objectManager();
    int type = obj_inst->getClassId(className());
    if (type != Type_none)
    {
        setPinType(paramOut(0), { className() });
        return true;
    }
    else
    {
        error = "no such class " + error;
        clearPinType(paramOut(0));
        return false;
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

    auto obj_inst = environment()->objectManager();
    auto meta = obj_inst->meta(class_name);
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

bool JZNodeCreateFromString::update(QString &error)
{
    int id = paramIn(0);
    
    auto obj_inst = environment()->objectManager();
    int type = obj_inst->getClassId(className());
    if (type != Type_none)
    {
        setPinType(paramOut(0), { className() });
        return true;
    }
    else
    {
        error = JZNodeCompiler::errorString(Error_noClass, { className() });
        clearPinType(paramOut(0));
        return false;
    }
}

void JZNodeCreateFromString::setClassName(const QString &name)
{
    setPinValue(paramIn(0), name);
}

QString JZNodeCreateFromString::className() const
{
    return pinValue(paramIn(0));
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
    m_name = "FunctionPointer";

    int out = addParamOut("",Pin_dispValue | Pin_editValue | Pin_literal);
    setPinType(out, { JZNodeType::typeName(Type_function) });
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

    auto func = environment()->functionManager()->function(function_name);
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

    int in = addParamIn("value",Pin_dispValue | Pin_widget);
    setPinTypeArg(in);

    addWidgetIn("Add input");
}

JZNodeDisplay::~JZNodeDisplay()
{

}

bool JZNodeDisplay::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeDebug(m_id);
    return true;
}

void JZNodeDisplay::addInput()
{
    auto pin0 = pin(paramIn(0));
    int in = addParamIn("", pin0->flag());    
    pin(in)->setDataType(pin0->dataType());
    if(pin0->isEditValue())
        pin(in)->setValue(0);
}

void JZNodeDisplay::removeInput(int index)
{
    int id = paramInList()[index];
    removePin(id);
}

JZNodePinWidget* JZNodeDisplay::createWidget(int id)
{
    Q_UNUSED(id);    

    auto in_list = paramInList();
    if(in_list.contains(id))
    {
        return new JZNodePinDisplayWidget(this,id);
    }
    else
    {
        JZNodePinButtonWidget *w = new JZNodePinButtonWidget(this, id);
        QPushButton *btn = w->button();
        btn->setText("Add Input");
        btn->connect(btn, &QPushButton::clicked, [this] {
            QByteArray old = toBuffer();
            addInput();
            propertyChangedNotify(old);        
        });                
        return w;
    }
}

QStringList JZNodeDisplay::pinActionList(int id)
{
    QStringList ret;
    if (paramInCount() > 2)
        ret.push_back("删除");

    return ret;
}

bool JZNodeDisplay::pinActionTriggered(int id, int index)
{
    int pin_index = paramInList().indexOf(id);
    removeInput(pin_index);
    return true;
}

bool JZNodeDisplay::canLink(int node_id, int pin_id, QString &error)
{
    if(m_file->getConnectPin(node_id,pin_id).size() > 1)
    {
        error = "只能连接一个变量";
        return false;
    }

    return true;
}

void JZNodeDisplay::onPinLinked(int id)
{
    widgetChangedNotify(id);
}

void JZNodeDisplay::onPinUnlinked(int id)
{
    widgetChangedNotify(id);
}

//JZNodePrint
JZNodePrint::JZNodePrint()
{
    m_type = Node_print;
    m_name = "print";

    addFlowIn();
    addFlowOut();
    int in = addParamIn("var", Pin_dispName | Pin_editValue);
    setPinTypeArg(in);
}

JZNodePrint::~JZNodePrint()
{
}

bool JZNodePrint::compiler(JZNodeCompiler *c,QString &error)
{
    if (c->pinInputCount(m_id,paramIn(0)) == 0 && !pin(paramIn(0))->value().isEmpty())
        c->setPinType(m_id,paramIn(0),Type_string);

    if (!c->addFlowInput(m_id, error))
        return false;

    auto in_id = irId(c->paramId(m_id, paramIn(0)));

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

    auto obj_inst = environment()->objectManager();
    QString className = class_file->className();
    int data_type = obj_inst->getClassId(className);
    Q_ASSERT(data_type != Type_none);
    setPinType(paramOut(0),{ className });
    return false;
}

//JZNodeParam
JZNodeParam::JZNodeParam()
{
    m_name = "get";
    m_flag = NodeProp_dragVariable;
    m_type = Node_param;
    
    int out = addParamOut("param", Pin_editValue);    
    setPinTypeString(out);
    setPinEditType(out, Type_paramName);
}

JZNodeParam::~JZNodeParam()
{
}

bool JZNodeParam::compiler(JZNodeCompiler *c,QString &error)
{    
    QString name = variable();    
    if (!c->checkVariableExist(name, error))
        return false;

    auto env = environment();
    auto def = c->getVariableInfo(name);    
    c->addNodeDebug(m_id);
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam ref = c->paramRef(name);    
    c->setPinType(m_id, paramOut(0), env->nameToType(def->type));
    c->addSetVariable(irId(out_id), ref);
    return true;
}

void JZNodeParam::setVariable(const QString &name)
{
    setPinValue(paramOut(0), name);
}

QString JZNodeParam::variable() const
{
    return pinValue(paramOut(0));
}

void JZNodeParam::drag(const QVariant &value)
{
    setVariable(value.toString());
}

bool JZNodeParam::update(QString &error)
{    
    QString name = variable();
    auto env = environment();
    auto def = JZNodeCompiler::getVariableInfo(m_file, name);
    int dataType = def ? env->nameToType(def->type) : Type_none;
    if (dataType != Type_none)
    {
        setPinType(paramOut(0), { def->type });
        return true;
    }
    {
        error = JZNodeCompiler::errorString(Error_noVariable, { name });
        clearPinType(paramOut(0));
        return false;
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
    setPinValue(paramIn(0), name);
}

QString JZNodeSetParam::variable() const
{
    return pinValue(paramIn(0));
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

bool JZNodeSetParam::update(QString &error)
{
    int id = paramIn(0);    
    auto env = environment();
    auto def = JZNodeCompiler::getVariableInfo(m_file, variable());        
    int dataType = def ? env->nameToType(def->type) : Type_none;
    if (dataType != Type_none)
    {
        setPinType(paramIn(1), { def->type });
        setPinType(paramOut(0), { def->type });
        return true;
    }
    else
    {
        error = JZNodeCompiler::errorString(Error_noVariable, { variable() });
        clearPinType(paramIn(1));
        clearPinType(paramOut(0));
        return false;
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
    setPinValue(paramIn(0),name);
}

QString JZNodeSetParamDataFlow::variable() const
{
    return pinValue(paramIn(0));
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

bool JZNodeSetParamDataFlow::update(QString &error)
{
    int id = paramIn(0);
    auto env = environment();
    auto def = JZNodeCompiler::getVariableInfo(m_file,variable());
    int dataType = def? env->nameToType(def->type) : Type_none;        
    if (dataType != Type_none)
    {
        setPinType(paramIn(1), { def->type });
        return true;
    }
    else
    {
        clearPinType(paramIn(1));
        error = JZNodeCompiler::errorString(Error_noVariable, { variable() });
        return false;
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
    addParamIn("class",Pin_dispName);    
}

JZNodeAbstractMember::~JZNodeAbstractMember()
{

}

void JZNodeAbstractMember::setClassName(QString className)
{
    setPinName(paramIn(0), className);
}

QString JZNodeAbstractMember::className()
{
    return pinName(paramIn(0));
}

void JZNodeAbstractMember::setMember(QString params)
{
    setPinValue(m_memberId, params);
}

QString JZNodeAbstractMember::member()
{
    return pinValue(m_memberId);
}

bool JZNodeAbstractMember::update(QString &error)
{
    auto class_name = className();
    auto env = environment();
    auto obj_inst = environment()->objectManager();
    auto meta = obj_inst->meta(class_name);
    if (!meta)
    {
        error = JZNodeCompiler::errorString(Error_noClass, { class_name });
        return false;
    }

    auto def = meta->param(member());
    if(!def || env->nameToType(def->type) == Type_none)
    {
        error = JZNodeCompiler::errorString(Error_classNoMember, { class_name,member() });
        return false;
    }
    m_memberType = def->type;
    setPinType(paramIn(0), { meta->className });
    return true;
}

//JZNodeMemberParam
JZNodeMemberParam::JZNodeMemberParam()
{
    m_name = "getMember";
    m_type = Node_memberParam;

    m_memberId = addParamOut("", Pin_editValue);
}

JZNodeMemberParam::~JZNodeMemberParam()
{

}

bool JZNodeMemberParam::update(QString &error)
{
    if (!JZNodeAbstractMember::update(error))
        return false;
    
    setPinType(paramOut(0), { m_memberType });
    return true;
}

bool JZNodeMemberParam::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addDataInput(m_id, error))
        return false;

    int in_id = c->paramId(m_id, paramIn(0));
    int out_id = c->paramId(m_id, paramOut(0));

    auto in = irId(in_id);
    in.member = member();    
    c->addSetVariable(irId(out_id), in);    
    return true;
}

//JZNodeSetMemberParam
JZNodeSetMemberParam::JZNodeSetMemberParam()
{
    m_name = "setMember";
    m_type = Node_setMemberParam;

    m_memberId = addParamIn("", Pin_editValue | Pin_noValue);
    addParamIn("value",Pin_dispValue);

    addFlowIn();
    addFlowOut();    
}

JZNodeSetMemberParam::~JZNodeSetMemberParam()
{

}

bool JZNodeSetMemberParam::update(QString &error)
{
    if (!JZNodeAbstractMember::update(error))
        return false;

    auto env = environment();
    auto pin_value = pin(paramIn(2));
    pin_value->setDataType({ m_memberType });    
    if (JZNodeType::isBaseOrEnum(env->nameToType(m_memberType)))
        pin_value->setFlag(pin_value->flag() | Pin_editValue);
    else
        pin_value->setFlag(pin_value->flag() & ~Pin_editValue);
    return true;
}

bool JZNodeSetMemberParam::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id,error))
        return false;

    int in_id = c->paramId(m_id, paramIn(0));
    int var_id = c->paramId(m_id, paramOut(2));

    auto in = irId(in_id);
    in.member = member();
    c->addSetVariable(irId(in_id), irId(var_id));
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
    setPinTypeArg(in);
    setPinTypeArg(out);
}

JZNodeClone::~JZNodeClone()
{

}

bool JZNodeClone::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addDataInput(m_id, error))
        return false;

    int in_type = c->pinType(m_id, paramIn(0));
    QList<JZNodeIRParam> in;
    QList<JZNodeIRParam> out;
    in << irId(c->paramId(m_id, paramIn(0)));
    out << irId(c->paramId(m_id, paramOut(0)));
    c->setPinType(m_id,paramOut(0),in_type);
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
    setPinTypeArg(in1);
    setPinTypeArg(in2);
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
            JZNodeFunction *node_func = dynamic_cast<JZNodeFunction*>(node);
            auto func_info = JZNodeUtils::splitMember(node_func->function());

            if (func_info.className.startsWith("QList<") && func_info.name == "get")
                return swap_list;
            else if (func_info.className.startsWith("QMap<") && func_info.name == "get")
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
                JZNodeFunction *node_func = dynamic_cast<JZNodeFunction*>(node);
                auto func_info = JZNodeUtils::splitMember(node_func->function());

                QList<JZNodeIRParam> in;
                QList<JZNodeIRParam> out;
                in << irId(c->paramId(node->id(), node->paramIn(0)));
                in << irId(c->paramId(node->id(), node->paramIn(1)));
                in << irId(id);
                c->addCall(func_info.className + ".set", in, out);
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