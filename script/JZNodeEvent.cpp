#include "JZNodeEvent.h"
#include "JZEvent.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeName.h"

// JZNodeEvent
JZNodeEvent::JZNodeEvent()
{
    m_type = Node_none;
    m_eventType = Event_none;

    addFlowOut();
}

JZNodeEvent::~JZNodeEvent()
{

}

void JZNodeEvent::saveToStream(JZProjectStream &s) const
{
    JZNode::saveToStream(s);
    s << m_eventType;
}

void JZNodeEvent::loadFromStream(JZProjectStream &s)
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

//JZNodeStartEvent
JZNodeStartEvent::JZNodeStartEvent()
{
    m_type = Node_startEvent;
    m_eventType = Event_programStart;
    m_name = "startProgram";
    setFlag(Node_propNoRemove);
}

FunctionDefine JZNodeStartEvent::function()
{
    FunctionDefine func;
    func.name = "__main__";
    func.isFlowFunction = true;
    return func;
}

bool JZNodeStartEvent::compiler(JZNodeCompiler *c, QString &error)
{
    auto func = function();
    c->addFunctionAlloc(func);    
    c->addJumpNode(flowOut());
    return true;
}


//JZNodeSingleEvent
JZNodeSingleEvent::JZNodeSingleEvent()
{
    m_type = Node_singleEvent;
    m_flag = Node_propDragVariable;
    int in = addParamIn("",Prop_dispName | Prop_dispValue | Prop_editValue | Prop_literal);
    setPinTypeString(in);
}

JZNodeSingleEvent::~JZNodeSingleEvent()
{

}

FunctionDefine JZNodeSingleEvent::function()
{    
    FunctionDefine def;
    def.isFlowFunction = true;    
    def.name = "on_" + JZNodeName::memberName(variable()) + "_" + JZNodeName::memberName(name());
    
    auto cls = m_file->getClassFile();
    if (cls)
    {
        def.className = cls->name();
        def.paramIn << JZParamDefine("this", cls->classType());
    }
    
    //存在私有信号，没有对应的函数，这里要取信号来计算
    auto meta = JZNodeObjectManager::instance()->meta(m_sender);
    auto s = meta->single(m_single);
    if(s->paramOut.size() > 0)
        def.paramIn << s->paramOut.mid(1);

    return def;
}

void JZNodeSingleEvent::setSingle(QString className,const SingleDefine *single)
{
    Q_ASSERT(single);

    auto def = JZNodeObjectManager::instance()->meta(className);
    Q_ASSERT(def);

    m_single = single->name;
    m_sender = className;

    QString function = className + "." + single->name;
    setName(function);
    setPropName(paramIn(0),className);
   
    for(int i = 1; i < single->paramOut.size(); i++)
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
    return propValue(paramIn(0));
}

int JZNodeSingleEvent::variableType() const
{
    auto def = JZNodeObjectManager::instance()->meta(m_sender);
    return def? def->id : Type_none;
}

void JZNodeSingleEvent::drag(const QVariant &value)
{
    setVariable(value.toString());
}

bool JZNodeSingleEvent::compiler(JZNodeCompiler *c,QString &error)
{    
    QString sender = propValue(paramIn(0));
    if(!c->checkVariableType(sender, m_sender,error))
        return false;
        
    auto def = JZNodeObjectManager::instance()->meta(m_sender);
    c->setPinType(m_id, paramIn(0), def->id);
    c->addFunctionAlloc(function());

    auto list = paramOutList();
    for(int i = 0; i < list.size(); i++)
    {
        int id = c->paramId(m_id,list[i]);
        c->addSetVariable(irId(id),irId(Reg_Call + 1 + i));
    }
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

void JZNodeSingleEvent::saveToStream(JZProjectStream &s) const
{
    JZNodeEvent::saveToStream(s);
    s << m_sender;
    s << m_single;
}

void JZNodeSingleEvent::loadFromStream(JZProjectStream &s)
{
    JZNodeEvent::loadFromStream(s);
    s >> m_sender;
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
    c->addFunctionAlloc(function());

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
void JZNodeQtEvent::saveToStream(JZProjectStream &s) const
{
    JZNodeEvent::saveToStream(s);
    s << m_className;
    s << m_event;
}

void JZNodeQtEvent::loadFromStream(JZProjectStream &s)
{
    JZNodeEvent::loadFromStream(s);
    s >> m_className;
    s >> m_event;
}

FunctionDefine JZNodeQtEvent::function()
{
    auto meta = JZNodeObjectManager::instance()->meta(m_className);
    auto def = meta->function(m_event);

    FunctionDefine func;
    func.isFlowFunction = true;
    func.className = m_className;    
    func.name = "event_" + m_event;
    func.paramIn = def->paramIn;
    return func;
}

void JZNodeQtEvent::setEvent(QString className, const EventDefine *func)
{
    Q_ASSERT(!className.isEmpty() && func);

    auto def = JZNodeObjectManager::instance()->meta(className);
    m_event = func->name;
    m_className = className;
    m_eventType = func->eventType;
    setName(func->name);

    auto paramOut = def->function(m_event)->paramIn;
    for (int i = 1; i < paramOut.size(); i++)
    {
        JZNodePin pin;
        pin.setName(paramOut[i].name);
        pin.setFlag(Prop_param | Prop_out);
        pin.setDataType({ paramOut[i].dataType });
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

FunctionDefine JZNodeParamChangedEvent::function()
{
    FunctionDefine def;
    return def;
}

bool JZNodeParamChangedEvent::compiler(JZNodeCompiler *compiler, QString &error)
{
    return false;
}