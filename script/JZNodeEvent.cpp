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

QList<JZParamDefine> JZNodeEvent::params()
{
    return QList<JZParamDefine>();
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
    addParamIn("",Prop_dispName | Prop_dispValue | Prop_editValue);
}

JZNodeSingleEvent::~JZNodeSingleEvent()
{

}

QList<JZParamDefine> JZNodeSingleEvent::params()
{
    auto def = JZNodeObjectManager::instance()->meta(m_className);
    auto sig_func = def->single(m_single);
    return sig_func->paramOut;    
}

void JZNodeSingleEvent::setSingle(QString className,const SingleDefine *single)
{
    Q_ASSERT(single);

    auto def = JZNodeObjectManager::instance()->meta(className);
    Q_ASSERT(def);

    m_single = single->name;
    m_className = className;

    QString function = className + "." + single->name;
    setName(function);
    setPropName(paramIn(0),className);
   
    for(int i = 0; i < single->paramOut.size(); i++)
    {
        JZNodePin pin;
        pin.setName(single->paramOut[i].name);
        pin.setFlag(Prop_param | Prop_out);
        pin.setDataType({single->paramOut[i].dataType});
        addProp(pin);
    }
    m_eventType = single->eventType;
}

QString JZNodeSingleEvent::single()
{
    return m_single;
}

void JZNodeSingleEvent::setVariable(const QString &name)
{
    setPropValue(paramIn(0),name);
}

QString JZNodeSingleEvent::variable() const
{
    return propValue(paramIn(0)).toString();
}

void JZNodeSingleEvent::drag(const QVariant &value)
{
    setVariable(value.toString());
}

bool JZNodeSingleEvent::compiler(JZNodeCompiler *c,QString &error)
{
    QString sender = propValue(paramIn(0)).toString();
    if(!c->checkVariableType(sender,m_className,error))
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

//JZNodeQtEvent
JZNodeQtEvent::JZNodeQtEvent()
{
    m_type = Node_qtEvent;    
}

JZNodeQtEvent::~JZNodeQtEvent()
{

}

bool JZNodeQtEvent::compiler(JZNodeCompiler *c, QString &error)
{    
    auto list = paramOutList();
    for (int i = 0; i < list.size(); i++)
    {
        int id = c->paramId(m_id, list[i]);
        c->addSetVariable(irId(id), irId(Reg_Call + i + 1));
    }
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;

}
void JZNodeQtEvent::saveToStream(QDataStream &s) const
{
    JZNodeEvent::saveToStream(s);
    s << m_className;
    s << m_event;
}

void JZNodeQtEvent::loadFromStream(QDataStream &s)
{
    JZNodeEvent::loadFromStream(s);
    s >> m_className;
    s >> m_event;
}

QList<JZParamDefine> JZNodeQtEvent::params()
{
    auto def = JZNodeObjectManager::instance()->meta(m_className);
    return def->event(m_event)->paramOut;
}

void JZNodeQtEvent::setEvent(QString className, const EventDefine *func)
{
    auto def = JZNodeObjectManager::instance()->meta(className);
    m_event = func->name;
    m_className = className;
    m_eventType = func->eventType;
    setName(func->name);

    for (int i = 1; i < func->paramOut.size(); i++)
    {
        JZNodePin pin;
        pin.setName(func->paramOut[i].name);
        pin.setFlag(Prop_param | Prop_out);
        pin.setDataType({ func->paramOut[i].dataType });
        addProp(pin);
    }
}

QString JZNodeQtEvent::event()
{
    return m_event;
}

//JZNodeParamChangedEvent
JZNodeParamChangedEvent::JZNodeParamChangedEvent()
{
    m_name = "ParamChanged";
    m_type = Node_paramChangedEvent;
    m_eventType = Event_paramChanged;

    addParamIn("", Prop_dispValue | Prop_editValue);
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
