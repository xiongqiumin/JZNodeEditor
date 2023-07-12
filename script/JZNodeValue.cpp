
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

int JZNodeLiteral::dataType()
{
    return prop(paramOut(0))->dataType()[0];
}

void JZNodeLiteral::setDataType(int type)
{
    prop(paramOut(0))->setDataType({type});
}

QVariant JZNodeLiteral::literal() const
{
    return propValue(paramOut(0));
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
    if(!c->addFlowInput(m_id,error))
        return false;

    int in_id = c->paramId(m_id,paramIn(0));
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam irIn = irId(in_id);
    JZNodeIRParam irOut = irId(out_id);

    c->addCall(irLiteral("createObject"),{irIn},{irOut});
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
    addParamIn("");
}

JZNodePrint::~JZNodePrint()
{
}

bool JZNodePrint::compiler(JZNodeCompiler *compiler,QString &error)
{
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
    QString className = m_file->bindClass();
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
    int out_id = c->paramId(m_id,paramOut(0));
    JZNodeIRParam ref = irRef(variable());
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
        auto def = m_file->getVariableInfo(variable());
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
        auto def = m_file->getVariableInfo(variable());
        int dataType = def? def->dataType : Type_none;
        setPinType(id,{dataType});
        setPinType(paramOut(0),{dataType});
    }
}

bool JZNodeSetParam::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addFlowInput(m_id,error))
        return false;

    int id = c->paramId(m_id,paramIn(0));
    int m_out = c->paramId(m_id,paramOut(0));

    JZNodeIRParam ref = irRef(variable());
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

    addParamIn("",Prop_dispName | Prop_editName | Prop_literal);
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
        auto def = m_file->getVariableInfo(variable());
        int dataType = def? def->dataType : Type_none;
        setPinType(id,{dataType});
    }
}

bool JZNodeSetParamDataFlow::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addDataInput(m_id,error))
        return false;

    QString param_name = variable();
    int id = c->paramId(m_id,paramIn(0));
    c->addSetVariable(irRef(param_name),irId(id));
    return true;
}
