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

//JZNodeStartEvent
JZNodeStartEvent::JZNodeStartEvent()
{
    m_type = Node_startEvent;
    m_eventType = Event_programStart;
    m_name = "startProgram";
    setFlag(NodeProp_noRemove);
}

JZFunctionDefine JZNodeStartEvent::function()
{
    JZFunctionDefine func;
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

//JZNodeFunctionStart
JZNodeFunctionStart::JZNodeFunctionStart()
{
    m_name = "Start";
    m_type = Node_functionStart;
    m_eventType = Event_functionStart;
    setFlag(NodeProp_noRemove);
}

JZNodeFunctionStart::~JZNodeFunctionStart()
{

}

JZFunctionDefine JZNodeFunctionStart::function()
{
    if (!m_file)
        return JZFunctionDefine();

    return m_file->function();
}

bool JZNodeFunctionStart::compiler(JZNodeCompiler *c, QString &error)
{
    c->addFunctionAlloc(m_file->function());
    c->addFlowOutput(m_id);
    c->addJumpNode(flowOut());
    return true;
}

//JZNodeSingleEvent
JZNodeSingleEvent::JZNodeSingleEvent()
{
    m_type = Node_singleEvent;
    m_flag = NodeProp_dragVariable;
    int in = addParamIn("",Pin_dispName | Pin_dispValue | Pin_editValue | Pin_literal);
    setPinTypeString(in);
}

JZNodeSingleEvent::~JZNodeSingleEvent()
{

}

JZFunctionDefine JZNodeSingleEvent::function()
{    
    JZFunctionDefine def;
    def.isFlowFunction = true;    
    def.name = "on_" + JZNodeName::memberName(variable()) + "_" + JZNodeName::memberName(name());
    
    auto cls = m_file->getClassFile();
    if (cls)
    {
        def.className = cls->name();
        def.paramIn << JZParamDefine("this", cls->classType());
    }
    
    //存在私有信号，没有对应的函数，这里要取信号来计算
    auto s = JZNodeObjectManager::instance()->single(m_single);    
    if(s->paramOut.size() > 0)
        def.paramIn << s->paramOut.mid(1);

    return def;
}

void JZNodeSingleEvent::setSingle(const SingleDefine *single)
{
    Q_ASSERT(single);

    auto def = JZNodeObjectManager::instance()->meta(single->className);
    Q_ASSERT(def);

    m_single = single->fullName();
    
    setName(m_single);
    setPinName(paramIn(0), single->className);
   
    for(int i = 1; i < single->paramOut.size(); i++)
    {
        JZNodePin pin;
        pin.setName(single->paramOut[i].name);
        pin.setFlag(Pin_param | Pin_out);
        pin.setDataType({single->paramOut[i].dataType()});
        addPin(pin);
    }
    m_eventType = single->eventType;
}

QString JZNodeSingleEvent::single()
{
    return m_single;
}

void JZNodeSingleEvent::setVariable(const QString &name)
{
    setPinValue(paramIn(0),JZNodeType::addMark(name));
}

QString JZNodeSingleEvent::variable() const
{
    return JZNodeType::removeMark(pinValue(paramIn(0)));
}

void JZNodeSingleEvent::drag(const QVariant &value)
{
    setVariable(value.toString());
}

bool JZNodeSingleEvent::compiler(JZNodeCompiler *c,QString &error)
{    
    QString className = m_single.split(".")[0];

    QString sender = variable();
    if(!c->checkVariableType(sender, className,error))
        return false;
        
    auto def = JZNodeObjectManager::instance()->meta(className);
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

void JZNodeSingleEvent::saveToStream(QDataStream &s) const
{
    JZNodeEvent::saveToStream(s);
    s << m_single;
}

void JZNodeSingleEvent::loadFromStream(QDataStream &s)
{
    JZNodeEvent::loadFromStream(s);
    s >> m_single;
}

//JZNodeSingleConnect
JZNodeSingleConnect::JZNodeSingleConnect()
{
    addFlowIn();
    addFlowOut();
    
    int in1 = addParamIn("sender", Pin_dispName);
    int in2 = addParamIn("signal", Pin_dispName | Pin_literal);
    int in3 = addParamIn("receiver", Pin_dispName);
    int in4 = addParamIn("slot", Pin_dispName | Pin_literal);

    setPinType(in1, { Type_object });
    setPinTypeString(in2);
    setPinType(in3, { Type_object });
    setPinTypeString(in4);
}

JZNodeSingleConnect::~JZNodeSingleConnect()
{

}

bool JZNodeSingleConnect::compiler(JZNodeCompiler *compiler, QString &error)
{
    return true;
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

JZFunctionDefine JZNodeQtEvent::function()
{
    auto meta = JZNodeObjectManager::instance()->meta(m_className);
    auto def = meta->function(m_event);

    JZFunctionDefine func;
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
        pin.setFlag(Pin_param | Pin_out);
        pin.setDataType({ paramOut[i].dataType() });
        addPin(pin);
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

    addParamIn("", Pin_dispValue | Pin_editValue);
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

JZFunctionDefine JZNodeParamChangedEvent::function()
{
    JZFunctionDefine def;
    return def;
}

bool JZNodeParamChangedEvent::compiler(JZNodeCompiler *compiler, QString &error)
{
    return false;
}