#include <QPushButton>
#include <QHBoxLayout>
#include <QLineEdit>
#include "JZNodeValue.h"
#include "JZNodeCompiler.h"
#include "JZProject.h"
#include "JZNodeFactory.h"
#include "JZClassItem.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeFunction.h"
#include "JZNodeUtils.h"
#include "JZFormat.h"

//JZNodeLiteral
JZNodeLiteral::JZNodeLiteral()
{
    m_type = Node_literal;
    m_name = "literal";
    addParamOut("out");    
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
    Q_ASSERT(JZNodeType::isBase(type));

    int out = paramOut(0);        
    pin(out)->setDataType({ JZNodeType::typeName(type) });
    pin(out)->setFlag(Pin_out | Pin_param);        
    if (type == Type_bool)
        setLiteral("false");
    else if (type == Type_int)
        setLiteral("0");
    else if (type == Type_int64)
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
    c->addNodeEnter(m_id);
    return true;
}

//JZNodeEnum
JZNodeEnum::JZNodeEnum()
{
    m_type = Node_enum;
    addParamOut("out");    
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
    c->addNodeEnter(m_id);

    c->setPinType(m_id,paramOut(0),def->id());
    return true;
}

void JZNodeEnum::setEnum(const JZNodeEnumDefine *meta)
{    
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
    addParamOut("out");
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
    c->addNodeEnter(m_id);

    c->setPinType(m_id, paramOut(0), def->id());
    return true;
}

void JZNodeFlag::setFlag(const JZNodeEnumDefine *meta)
{
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

    int in = addParamIn("var");
    setPinTypeArg(in);
    addParamOut("");
}

JZNodeConvert::~JZNodeConvert()
{
}

void JZNodeConvert::setOutputType(int type)
{
    auto env = environment();
    QString name = env->typeToName(type);
    setPinValue(paramOut(0), name);
    
    update();
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
    c->addConvert(irId(out_id),out_type,irId(in_id));
    return true;
}

bool JZNodeConvert::updateNode(QString &error)
{
    auto env = environment();
    int id = paramOut(0);
    QString name = pinValue(id);
    int type = env->nameToType(name);
        
    if (type != Type_none)
    {
        setPinType(paramOut(0), { name });
        return true;
    }
    else
    {
        error = JZNodeCompiler::errorString(Error_noType,{ name });
        clearPinType(paramOut(0));
        return false;
    }
}

//JZNodeCreateObject
JZNodeCreateObject::JZNodeCreateObject()
{
    m_name = "createObject";
    m_type = Node_create;    

    int id = addParamIn("Class",Pin_constValue | Pin_noCompiler);
    setPinTypeString(id);
    addParamOut("Return");

    setPinTypeString(id);
}

JZNodeCreateObject::~JZNodeCreateObject()
{

}

void JZNodeCreateObject::setClassName(const QString &name)
{
    setPinValue(paramIn(0), name);
}

QString JZNodeCreateObject::className() const
{
    return pinValue(paramIn(0));    
}

bool JZNodeCreateObject::compiler(JZNodeCompiler *c,QString &error)
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

bool JZNodeCreateObject::updateNode(QString &error)
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

    int in1 = addParamIn("Class", Pin_constValue);
    int in2 = addParamIn("Context");
    setPinTypeString(in1);
    setPinTypeString(in2);
    addParamOut("Return");
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

bool JZNodeCreateFromString::updateNode(QString &error)
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
        error = JZNodeCompiler::errorString(Error_noType, { className() });
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

    int out = addParamOut("", Pin_constValue);
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

    JZFunctionPointer ptr(function_name);
    c->addSetVariable(irId(id), irLiteral(QVariant::fromValue(ptr)));
    c->addNodeEnter(m_id);

    return true;
}

//JZNodeFormatImpl
JZNodeFormatImpl::JZNodeFormatImpl()
{
    m_type = Node_format;
    m_name = "format";
    m_argIndex = 0;

    int in = addParamIn("format", Pin_constValue | Pin_noCompiler);
    setPinTypeString(in);
}

JZNodeFormatImpl::~JZNodeFormatImpl()
{
}

void JZNodeFormatImpl::setFormat(QString format)
{
    setParamInValue(0, format);
}

QString JZNodeFormatImpl::format()
{
    return paramInValue(0);
}

bool JZNodeFormatImpl::updateNode(QString& error)
{
    QString format_str = this->format();

    int brace_count = -1;
    if (m_isText)
    {
        JZFormat format;
        if (!format.init(format_str, error))
            return false;

        brace_count = format.paramCount();
    }
    else
    {
        JZFormatBinary format;
        if (!format.init(format_str, error))
            return false;

        brace_count = format.paramCount();
    }

    paramInResize(m_argIndex + brace_count);
    auto list = paramInList();
    for (int i = m_argIndex; i < list.size(); i++)
        setPinName(list[i], "in" + QString::number(i));

    return true;
}

bool JZNodeFormatImpl::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QList<JZNodeIRParam> in, out;
    auto param_in_list = paramInList();
    in << irLiteral(format());
    for (int i = 0; i < param_in_list.size(); i++)
        in << irId(c->paramId(m_id, paramIn(0)));
    
    out << irId(c->paramId(m_id, paramOut(0)));
    c->addCall(m_function, in, out);

    return true;
}


//JZNodeFormat
JZNodeFormat::JZNodeFormat()
{
    m_type = Node_format;
    m_name = "format";
    m_function = "JZFormat";

    addParamOut("out");
    setPinTypeString(paramOut(0));
}

JZNodeFormat::~JZNodeFormat()
{
}

//JZNodeFormatBin
JZNodeFormatBin::JZNodeFormatBin()
{
    m_type = Node_formatBin;
    m_name = "formatBin";
    m_function = "JZFormatBin";

    addParamOut("out");
    setPinType(paramOut(0), { JZNodeType::typeName(Type_byteArray) });
}

JZNodeFormatBin::~JZNodeFormatBin()
{
}

//JZNodePrint
JZNodePrint::JZNodePrint()
{
    m_type = Node_print;
    m_name = "print";
    m_function = "JZPrint";

    addFlowIn();
    addFlowOut();
}

JZNodePrint::~JZNodePrint()
{
}

//JZNodeLog
JZNodeLog::JZNodeLog()
{
    m_type = Node_log;
    m_name = "log";
    m_function = "JZLog";

    int m = addParamIn("module");
    int l = addParamIn("level");
    setPinTypeInt(m);
    setPinTypeInt(l);

    addFlowIn();
    addFlowOut();
}

JZNodeLog::~JZNodeLog()
{
}


//JZNodeDisplay
JZNodeDisplay::JZNodeDisplay()
{
    m_type = Node_display;
    m_name = "diaplay";

    addInput();
}

JZNodeDisplay::~JZNodeDisplay()
{
}

void JZNodeDisplay::addInput()
{
    int in = addParamIn("input");
    setPinType(in,{ "arg" });
}

bool JZNodeDisplay::compiler(JZNodeCompiler *compiler, QString &error)
{
    Q_ASSERT(0);
    return true;
}

void JZNodeDisplay::saveToStream(QDataStream &s) const
{
    QList<JZNodePin> &pin_list = const_cast<QList<JZNodePin>&>(m_pinList);

    QStringList values;
    for(int i = 0; i < pin_list.size(); i++)
    {
        values << pin_list[i].value();
        pin_list[i].setValue(QString());
    }

    JZNode::saveToStream(s);

    for(int i = 0; i < pin_list.size(); i++)
        pin_list[i].setValue(values[i]);
}

void JZNodeDisplay::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
}

//JZNodeThis
JZNodeThis::JZNodeThis()
{
    m_name = "this";
    m_type = Node_this;
    addParamOut("this");
}

JZNodeThis::~JZNodeThis()
{

}

bool JZNodeThis::compiler(JZNodeCompiler *c,QString &error)
{
    int out_id = c->paramId(m_id, paramOut(0));
    c->addNodeEnter(m_id);    
    c->addSetVariable(irId(out_id),irThis());    
    return true;
}

bool JZNodeThis::updateNode(QString &error)
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
    m_type = Node_param;
    
    int out = addParamOut("name");    
    setPinTypeString(out);
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
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam ref = c->paramRef(name);

    c->setIRParamReference(irId(out_id), ref);
    c->addNodeEnter(m_id);   
    if (def->type == "auto")
    {
        int data_type = c->refType(name);
        if (data_type == Type_auto)
        {
            error = "类型未定义";
            return false;
        }
        c->setPinType(m_id, paramOut(0), data_type);
    }
    else
    {
        c->setPinType(m_id, paramOut(0), env->nameToType(def->type));
    }
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

bool JZNodeParam::updateNode(QString &error)
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
    m_name = "set";

    addFlowIn();    
    addFlowOut();

    int in = addParamIn("name", Pin_constValue | Pin_noCompiler);
    setPinTypeString(in);

    addParamIn("value");   
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

void JZNodeSetParam::setValue(const QString &value)
{
    setPinValue(paramIn(1), value);
}

QString JZNodeSetParam::value() const
{
    return pinValue(paramIn(1));
}

bool JZNodeSetParam::updateNode(QString &error)
{
    int id = paramIn(0);    
    auto env = environment();
    auto def = JZNodeCompiler::getVariableInfo(m_file, variable());        
    int dataType = def ? env->nameToType(def->type) : Type_none;
    if (dataType != Type_none)
    {
        setPinType(paramIn(1), { def->type });
        return true;
    }
    else
    {
        error = JZNodeCompiler::errorString(Error_noVariable, { variable() });
        clearPinType(paramIn(1));
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
    
    JZNodeIRParam ref = c->paramRef(name);
    if (c->refType(name) == Type_auto)
    {
        auto in_type = c->pinType(m_id, paramIn(1));
        c->setRefType(name, in_type);
    }

    c->addSetVariable(ref,irId(id));
    c->addFlowOutput(m_id);
    return true;
}

//JZNodeMemberParam
JZNodeMemberParam::JZNodeMemberParam()
{
    m_name = "getMember";
    m_type = Node_memberParam;

    addParamIn("object");
    addParamOut("");
}

JZNodeMemberParam::~JZNodeMemberParam()
{

}

bool JZNodeMemberParam::update(QString &error)
{
    return true;
}

bool JZNodeMemberParam::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addDataInput(m_id, error))
        return false;

    int in_id = c->paramId(m_id, paramIn(0));
    int out_id = c->paramId(m_id, paramOut(0));

    auto in = irId(in_id);
    c->addSetVariable(irId(out_id), in);    
    return true;
}

//JZNodeSetMemberParam
JZNodeSetMemberParam::JZNodeSetMemberParam()
{
    m_name = "setMember";
    m_type = Node_setMemberParam;

    addParamIn("object");
    addParamIn("");

    addFlowIn();
    addFlowOut();    
}

JZNodeSetMemberParam::~JZNodeSetMemberParam()
{

}

bool JZNodeSetMemberParam::update(QString &error)
{
    return true;
}

bool JZNodeSetMemberParam::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addFlowInput(m_id,error))
        return false;

    int in_id = c->paramId(m_id, paramIn(0));
    int var_id = c->paramId(m_id, paramOut(2));

    c->addSetVariable(irId(in_id), irId(var_id));
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
                c->addCall(func_info.className + "::set", in, out);
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
    
    return true;
}