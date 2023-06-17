#include "JZNodeEvent.h"
#include "JZEvent.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"

// JZNodeEvent
JZNodeEvent::JZNodeEvent()
{
    m_type = Node_event;
    m_eventType = Event_none;

    addFlowOut();
}

JZNodeEvent::~JZNodeEvent()
{

}

void JZNodeEvent::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
    s << m_eventType;
}

void JZNodeEvent::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
    s >> m_eventType;
}

void JZNodeEvent::setEventType(int eventType)
{
    m_eventType = eventType;            
}

int JZNodeEvent::eventType() const
{
    return m_eventType;
}

QList<FunctionParam> JZNodeEvent::params()
{
    return QList<FunctionParam>();
}

bool JZNodeEvent::compiler(JZNodeCompiler *c,QString &error)
{    
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeSingleEvent
JZNodeSingleEvent::JZNodeSingleEvent()
{
    m_type = Node_singleEvent;
    m_flag = Node_propVariable;
    addParamIn("",Prop_dispName | Prop_dispValue | Prop_edit);
}

JZNodeSingleEvent::~JZNodeSingleEvent()
{

}

void JZNodeSingleEvent::setVariable(const QString &sender)
{
    setPropValue(paramIn(0),sender);
}

QString JZNodeSingleEvent::variable() const
{
    return propValue(paramIn(0)).toString();
}

QList<FunctionParam> JZNodeSingleEvent::params()
{
    auto def = JZNodeObjectManager::instance()->meta(m_className);
    auto sig_func = def->function(single());
    return sig_func->paramIn;
}

void JZNodeSingleEvent::setSingle(QString className,const SingleDefine *single)
{
    auto inst = JZNodeFunctionManager::instance();
    QString function = single->name;
    m_single = single->name;
    if(!className.isEmpty())
    {
        m_className = className;
        addParamIn(m_className,Prop_dispName | Prop_dispValue | Prop_edit);
        function = className + "." + function;
    }
    setName(function);

    const FunctionDefine *func = inst->function(function);
    for(int i = 0; i < func->paramIn.size(); i++)
    {
        JZNodePin pin;
        pin.setName(func->paramIn[i].name);
        pin.setFlag(Prop_param | Prop_out);
        pin.setDataType({func->paramIn[i].dataType});
        addProp(pin);
    }
    m_eventType = single->eventType;
}

QString JZNodeSingleEvent::single()
{
    return m_single;
}

bool JZNodeSingleEvent::compiler(JZNodeCompiler *c,QString &error)
{
    QString sender = propValue(paramIn(0)).toString();
    if(!c->checkVariable(sender,m_className,error))
        return false;

    auto list = paramOutList();
    for(int i = 0; i < list.size(); i++)
    {
        int id = c->paramId(m_id,list[i]);
        c->addSetVariable(irId(id),irId(Reg_Call + i));
    }
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

void JZNodeSingleEvent::saveToStream(QDataStream &s) const
{
    JZNodeEvent::saveToStream(s);
    s << m_className;
    s << m_single;
}

void JZNodeSingleEvent::loadFromStream(QDataStream &s)
{
    JZNodeEvent::loadFromStream(s);
    s >> m_className;
    s >> m_single;
}

//JZNodeParamChangedEvent
JZNodeParamChangedEvent::JZNodeParamChangedEvent()
{
    m_name = "ParamChanged";
    m_type = Node_paramChangedEvent;
    m_eventType = Event_paramChanged;

    addParamIn("", Prop_dispValue | Prop_edit);
}

JZNodeParamChangedEvent::~JZNodeParamChangedEvent()
{

}

void JZNodeParamChangedEvent::setVariable(const QString &name)
{

}

QString JZNodeParamChangedEvent::variable() const
{
    return QString();
}
