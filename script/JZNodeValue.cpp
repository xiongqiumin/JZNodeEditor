#include "JZNodeValue.h"
#include "JZNodeCompiler.h"
#include "JZProject.h"

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
    prop(paramOut(0))->setDataType({type});
    if (type == Type_nullptr)
    {
        int out = paramOut(0);
        setPropName(out, "null");
        prop(out)->setFlag(Prop_out | Prop_param  | Prop_dispName);
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
        v.convert(JZNodeType::typeToQMeta(dataType));
    return v;
}

void JZNodeLiteral::setLiteral(QVariant value)
{
    setPropValue(paramOut(0),value);
}

bool JZNodeLiteral::compiler(JZNodeCompiler *c,QString &error)
{   
    int id = c->paramId(m_id,paramOut(0));
    c->addSetVariable(irId(id),irLiteral(literal()));
    return true;
}

//JZNodeCreate
JZNodeCreate::JZNodeCreate()
{
    m_name = "createObject";
    m_type = Node_create;

    addFlowIn();
    addFlowOut();

    int id = addParamIn("Class",Prop_editValue | Prop_dispName | Prop_dispValue | Prop_literal);
    addParamOut("Return", Prop_dispName);

    setPinTypeString(id);
}

JZNodeCreate::~JZNodeCreate()
{

}

void JZNodeCreate::setClassName(const QString &name)
{
    setPropValue(paramIn(0),name);
}

QString JZNodeCreate::className() const
{
    return propValue(paramIn(0)).toString();
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

//JZNodePrint
JZNodePrint::JZNodePrint()
{
    m_type = Node_print;
    m_name = "Print";

    addFlowIn();
    addFlowOut();
    int in = addParamIn("");
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
    int out_id = c->paramId(m_id,paramOut(0));
    c->addSetVariable(irId(out_id),irThis());
    return true;
}

void JZNodeThis::fileInitialized()
{
    auto class_file = m_file->project()->getClassFile(m_file);
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
        int dataType = def? def->dataType : Type_none;
        setPinType(id,{dataType});
    }
}

//JZNodeSetParam
JZNodeSetParam::JZNodeSetParam()
{
    m_type = Node_setParam;
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
        int dataType = def? def->dataType : Type_none;
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
    m_type = Node_setParamData;

    addParamIn("",Prop_dispName | Prop_editName);
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
        int dataType = def? def->dataType : Type_none;
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
