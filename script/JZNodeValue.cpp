#include "JZNodeValue.h"
#include "JZNodeCompiler.h"
#include "JZProject.h"
#include "JZNodeFactory.h"
#include "JZClassItem.h"

//JZNodeLiteral
JZNodeLiteral::JZNodeLiteral()
{
    m_type = Node_literal;
    addParamOut("out", Prop_dispValue | Prop_editValue);
    prop(paramOut(0))->setDataType({Type_none});
}

JZNodeLiteral::~JZNodeLiteral()
{
}

int JZNodeLiteral::dataType() const
{
    return prop(paramOut(0))->dataType()[0];
}

void JZNodeLiteral::setDataType(int type)
{
    int out = paramOut(0);
    prop(out)->setDataType({type});
    setName(JZNodeType::typeToName(type));
    if(JZNodeType::isBaseOrEnum(type))
        prop(out)->setFlag(Prop_out | Prop_param | Prop_dispValue | Prop_editValue);
    else
        prop(out)->setFlag(Prop_out | Prop_param | Prop_dispName);
    
    if (type == Type_bool)
        setLiteral(false);
    else if (type == Type_int)
        setLiteral(0);
    else if (type == Type_double)
        setLiteral(0.0);
    else if (type == Type_string)
        setLiteral("");    
    else if (type == Type_nullptr)
    {
        setPropName(out, "null");
        setLiteral(QVariant::fromValue(JZObjectNull()));
    }
}

QVariant JZNodeLiteral::literal() const
{
    auto pin = prop(paramOut(0));
    QVariant v = pin->value();
    int dataType = pin->dataType()[0];
    if (dataType == Type_nullptr)
        v = QVariant::fromValue(JZObjectNull());
    else
        v = JZNodeType::matchValue(dataType, v);        

    return v;
}

void JZNodeLiteral::setLiteral(QVariant value)
{
    QString str = JZNodeType::matchValue(Type_string, value).toString();
    setPropValue(paramOut(0), str);
}

bool JZNodeLiteral::compiler(JZNodeCompiler *c,QString &error)
{   
    int id = c->paramId(m_id,paramOut(0));
    c->addNodeStart(m_id);
    c->addSetVariable(irId(id),irLiteral(literal()));
    return true;
}

//JZNodeEnum
JZNodeEnum::JZNodeEnum()
{
    m_type = Node_enum;
    addParamOut("out", Prop_dispValue | Prop_editValue);
    m_enumId = -1;
}

JZNodeEnum::~JZNodeEnum()
{

}

bool JZNodeEnum::compiler(JZNodeCompiler *c, QString &error)
{
    QString key = prop(paramOut(0))->value();
    auto def = JZNodeObjectManager::instance()->enumMeta(m_name);
    int v = def->keyToValue(key);

    c->addNodeStart(m_id);
    int id = c->paramId(m_id, paramOut(0));
    c->addSetVariable(irId(id), irLiteral(v));
    c->lastStatment()->memo = key;
        
    c->setPinType(m_id,paramOut(0),def->type());
    return true;
}

void JZNodeEnum::setEnum(int id)
{
    auto meta = JZNodeObjectManager::instance()->enumMeta(id);
    m_enumId = id;

    setName(meta->name());
    setPinType(paramOut(0), { id });
    setEnumValue(meta->value(0));
}

void JZNodeEnum::setEnumValue(int value)
{
    auto meta = JZNodeObjectManager::instance()->enumMeta(m_enumId);
    prop(paramOut(0))->setValue(meta->valueToKey(value));
}

//JZNodeCreate
JZNodeCreate::JZNodeCreate()
{
    m_name = "createObject";
    m_type = Node_create;

    addFlowIn();
    addFlowOut();

    int id = addParamIn("Class",Prop_editValue | Prop_dispName | Prop_dispValue | Prop_literal);
    setPinTypeString(id);
    addParamOut("Return", Prop_dispName);

    setPinTypeString(id);
}

JZNodeCreate::~JZNodeCreate()
{

}

void JZNodeCreate::setClassName(const QString &name)
{
    setPropValue(paramIn(0),"\"" + name + "\"");
}

QString JZNodeCreate::className() const
{
    QString name = propValue(paramIn(0));
    if (name.front() == '\"' && name.back() == '\"')
        return name.mid(1, name.size() - 2);
    else
        return QString();
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
    if(!c->addFlowInput(m_id,error))
        return false;

    int in_id = c->paramId(m_id,paramIn(0));
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam irIn = irId(in_id);
    JZNodeIRParam irOut = irId(out_id);

    c->addCall(irLiteral("createObject"), { irIn }, { irOut });    
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

void JZNodeCreate::pinChanged(int id)
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

    addFlowIn();
    addFlowOut();

    int in1 = addParamIn("Class", Prop_editValue | Prop_dispName | Prop_dispValue | Prop_literal);
    int in2 = addParamIn("Context", Prop_editValue | Prop_dispName | Prop_dispValue );
    setPinTypeString(in1);
    setPinTypeString(in2);
    addParamOut("Return", Prop_dispName);
}

JZNodeCreateFromString::~JZNodeCreateFromString()
{

}

bool JZNodeCreateFromString::compiler(JZNodeCompiler *c, QString &error)
{
    if (className().isEmpty())
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
    if (!c->addFlowInput(m_id, error))
        return false;

    int in_id = c->paramId(m_id, paramIn(0));
    int out_id = c->paramId(m_id, paramOut(0));
    QList<JZNodeIRParam> in, out;
    in << irId(in_id) << irId(c->paramId(m_id, paramIn(1)));
    out << irId(out_id);

    c->addCall(irLiteral("createObjectFromString"), in, out);
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;

}

void JZNodeCreateFromString::pinChanged(int id)
{
    if (id == paramIn(0))
    {
        int type = JZNodeObjectManager::instance()->getClassId(className());
        setPinType(paramOut(0), { type });
    }
}

void JZNodeCreateFromString::setClassName(const QString &name)
{
    setPropValue(paramIn(0), "\"" + name + "\"");
}

QString JZNodeCreateFromString::className() const
{
    QString name = propValue(paramIn(0));
    if (name.front() == '\"' && name.back() == '\"')
        return name.mid(1, name.size() - 2);
    else
        return QString();
}

void JZNodeCreateFromString::setContext(const QString &text)
{
    setPropValue(paramIn(1), text);
}

QString JZNodeCreateFromString::context() const
{
    return propValue(paramIn(1));
}

//JZNodeParamFunction
JZNodeParamFunction::JZNodeParamFunction()
{

}

JZNodeParamFunction::~JZNodeParamFunction()
{

}

bool JZNodeParamFunction::compiler(JZNodeCompiler *compiler, QString &error)
{
    return false;
}

//JZNodeDisplay
JZNodeDisplay::JZNodeDisplay()
{
    m_type = Node_display;
    m_name = "display";

    int in = addParamIn("value",Prop_dispValue);
    setPinTypeAny(in);
}

JZNodeDisplay::~JZNodeDisplay()
{

}

bool JZNodeDisplay::compiler(JZNodeCompiler *c, QString &error)
{
    c->addNodeStart(m_id);
    return true;
}

//JZNodePrint
JZNodePrint::JZNodePrint()
{
    m_type = Node_print;
    m_name = "print";

    addFlowIn();
    addFlowOut();
    int in = addParamIn("text", Prop_dispName);
    setPinTypeAny(in);
}

JZNodePrint::~JZNodePrint()
{
}

bool JZNodePrint::compiler(JZNodeCompiler *c,QString &error)
{
    if (!c->addFlowInput(m_id, error))
        return false;
    
    QList<JZNodeIRParam> in, out;
    in << irId(c->paramId(m_id, paramIn(0)));
    c->addCall(irLiteral("print"), in, out);    
    c->addJumpNode(flowOut());

    return true;
}

//JZNodeThis
JZNodeThis::JZNodeThis()
{
    m_name = "this";
    m_type = Node_this;
    addParamOut("this",Prop_dispName);
}

JZNodeThis::~JZNodeThis()
{

}

bool JZNodeThis::compiler(JZNodeCompiler *c,QString &error)
{
    int out_id = c->paramId(m_id, paramOut(0));
    c->addNodeStart(m_id);    
    c->addSetVariable(irId(out_id),irThis());    
    return true;
}

void JZNodeThis::fileInitialized()
{
    auto class_file = m_file->project()->getItemClass(m_file);
    Q_ASSERT(class_file);

    QString className = class_file->className();
    int data_type = JZNodeObjectManager::instance()->getClassId(className);
    Q_ASSERT(data_type != Type_none);
    setPinType(paramOut(0),{data_type});
}

//JZNodeParam
JZNodeParam::JZNodeParam()
{
    m_name = "get";
    m_flag = Node_propDragVariable;
    m_type = Node_param;
    addParamOut("",Prop_dispName | Prop_editName);
}

JZNodeParam::~JZNodeParam()
{
}

bool JZNodeParam::compiler(JZNodeCompiler *c,QString &error)
{    
    QString name = variable();
    if (!c->checkVariableExist(name, error))
        return false;
    
    c->addNodeStart(m_id);
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam ref = irRef(name);
    c->addSetVariable(irId(out_id),ref);
    return true;
}

void JZNodeParam::setVariable(const QString &name)
{
    setPropName(paramOut(0),name);
}

QString JZNodeParam::variable() const
{
    return propName(paramOut(0));
}

void JZNodeParam::drag(const QVariant &value)
{
    setVariable(value.toString());
}

void JZNodeParam::pinChanged(int id)
{
    if(id == paramOut(0))
    {
        auto def = JZNodeCompiler::getVariableInfo(m_file, variable());
        int dataType = def? def->dataType() : Type_none;        
        setPinType(id,{dataType});
    }
}

//JZNodeSetParam
JZNodeSetParam::JZNodeSetParam()
{
    m_type = Node_setParam;
    m_flag = Node_propDragVariable;
    m_name = "set";

    addFlowIn();    
    addFlowOut();

    addParamIn("",Prop_dispName | Prop_editName | Prop_editValue | Prop_dispValue);
    addParamOut("");
}

JZNodeSetParam::~JZNodeSetParam()
{
}

void JZNodeSetParam::setVariable(const QString &name)
{
    setPropName(paramIn(0),name);
}

QString JZNodeSetParam::variable() const
{
    return propName(paramIn(0));
}

void JZNodeSetParam::drag(const QVariant &value)
{
    setVariable(value.toString());
}

void JZNodeSetParam::pinChanged(int id)
{
    if(id == paramIn(0))
    {
        auto def = JZNodeCompiler::getVariableInfo(m_file, variable());
        int dataType = def? def->dataType() : Type_none;
        setPinType(id,{dataType});
        setPinType(paramOut(0),{dataType});
    }
}

bool JZNodeSetParam::compiler(JZNodeCompiler *c,QString &error)
{
    QString name = variable();
    if (!c->checkVariableExist(name, error))
        return false;
    if(!c->addFlowInput(m_id,error))
        return false;
        
    int id = c->paramId(m_id,paramIn(0));
    int m_out = c->paramId(m_id,paramOut(0));
    
    JZNodeIRParam ref = irRef(name);
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
    m_flag = Node_propDragVariable;
    m_type = Node_setParamData;

    addParamIn("",Prop_dispName | Prop_editName | Prop_editValue | Prop_dispValue);
}

JZNodeSetParamDataFlow::~JZNodeSetParamDataFlow()
{
}

void JZNodeSetParamDataFlow::setVariable(const QString &name)
{
    setPropName(paramIn(0),name);
}

QString JZNodeSetParamDataFlow::variable() const
{
    return propName(paramIn(0));
}

void JZNodeSetParamDataFlow::drag(const QVariant &value)
{
    setVariable(value.toString());
}

void JZNodeSetParamDataFlow::pinChanged(int id)
{
    if(id == paramIn(0))
    {
        auto def = JZNodeCompiler::getVariableInfo(m_file,variable());
        int dataType = def? def->dataType() : Type_none;
        setPinType(id,{dataType});
    }
}

bool JZNodeSetParamDataFlow::compiler(JZNodeCompiler *c,QString &error)
{
    QString name = variable();
    if (!c->checkVariableExist(name, error))
        return false;
    if(!c->addDataInput(m_id,error))
        return false;
    
    int id = c->paramId(m_id,paramIn(0));
    c->addSetVariable(irRef(name),irId(id));    
    return true;
}

//JZNodeAbstractMember
void JZNodeAbstractMember::setMember(QString className, QStringList param_list)
{
    auto meta = JZNodeObjectManager::instance()->meta(className);
    prop(paramIn(0))->setName(className);    
    setPinType(paramIn(0), { meta->id });

    for (int i = 0; i < param_list.size(); i++)
    {
        auto param = meta->param(param_list[i]);

        int flag = Prop_dispName;        
        int pin_id = -1;
        if (m_type == Node_setMemberParam || m_type == Node_setMemberParamData)
        {
            if (JZNodeType::isBaseOrEnum(param->dataType()))
                flag |= (Prop_dispValue | Prop_editValue);
            pin_id = addParamIn(param->name, flag);
        }
        else
        {            
            pin_id = addParamOut(param->name, flag);
        }
        setPinType(pin_id, { param->dataType() });
    }
}

QString JZNodeAbstractMember::className()
{
    return prop(paramIn(0))->name();
}

QStringList JZNodeAbstractMember::members()
{
    QStringList ret;
    QVector<int> pin_list;
    if (m_type == Node_setMemberParam || m_type == Node_setMemberParamData)
    {
        pin_list = paramInList();
        pin_list.removeAt(0);
    }
    else
        pin_list = paramOutList();

    for (int i = 0; i < pin_list.size(); i++)
        ret << prop(pin_list[i])->name();
    return ret;
}

bool JZNodeAbstractMember::pinClicked(int id)
{
    Q_UNUSED(id);
    return JZNodeFactory::instance()->edit(this);
}

//JZNodeMemberParam
JZNodeMemberParam::JZNodeMemberParam()
{
    m_name = "getMember";
    m_type = Node_memberParam;

    addParamIn("", Prop_dispName);
    addButtonIn("edit");
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
        in << irLiteral(prop(list[i])->name());
        out << irId(c->paramId(m_id, list[i]));
        c->addCall(irLiteral("getMemberParam"), in, out);
    }
    return true;
}

//JZNodeSetMemberParam
JZNodeSetMemberParam::JZNodeSetMemberParam()
{
    m_name = "setMember";
    m_type = Node_setMemberParam;

    addParamIn("", Prop_dispName);
    addFlowIn();
    addFlowOut();
    addButtonIn("edit");
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
        QString var_name = prop(paramIn(i + 1))->name();
        in[1] = irId(var_id);
        in[2] = irLiteral(var_name);
        c->addCall(irLiteral("setMemberParam"), in, out);    
    }

    c->addJumpNode(flowOut());
    return true;
}

//JZNodeSetMemberData
JZNodeSetMemberParamData::JZNodeSetMemberParamData()
{
    m_name = "setMember";
    m_type = Node_setMemberParamData;

    addParamIn("",Prop_dispName);    
    addButtonIn("edit");
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
    in << irLiteral(prop(paramIn(1))->name());
    in << irId(var_id);
    c->addCall(irLiteral("setMemberParam"), in, out);

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
    c->addCall(irLiteral("clone"), in, out);
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
                    c->addCall(irLiteral("List.set"), in, out);
                }
                else if (type == swap_map)
                {
                    c->addCall(irLiteral("Map.set"), in, out);
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