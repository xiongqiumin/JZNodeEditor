#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include "JZNodeEvent.h"
#include "JZNodeCompiler.h"
#include "JZNodeFunctionManager.h"
#include "JZNodeBind.h"
#include "JZNodeJson.h"

//JZNodeSignalConnect
JZNodeSignalConnect::JZNodeSignalConnect()
{
    m_type = Node_signalConnect;
    m_name = "connect";

    addFlowIn();
    addFlowOut();

    int in1 = addParamIn("sender");
    int in2 = addParamIn("signal", Pin_constValue);
    int in3 = addParamIn("receiver");
    int in4 = addParamIn("slot", Pin_constValue);

    setPinType(in1, { JZNodeType::typeName(Type_object) });
    setPinType(in2, { JZNodeType::typeName(Type_function) });
    setPinType(in3, { JZNodeType::typeName(Type_object) });
    setPinType(in4, { JZNodeType::typeName(Type_function) });
}

JZNodeSignalConnect::~JZNodeSignalConnect()
{

}

bool JZNodeSignalConnect::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addFlowInput(m_id, error))
        return false;

    QString sig = c->pinLiteral(m_id, paramIn(1));
    QString slot = c->pinLiteral(m_id, paramIn(3));

    auto env = environment();
    auto sig_func = env->objectManager()->signal(sig);
    auto slot_func = env->functionManager()->function(slot);
    if (!sig_func)
    {
        error = "no signal " + sig;
        return false;
    }
    if (!slot_func)
    {
        error = "no slot " + slot;
        return false;
    }
    if (!JZNodeType::sigSlotTypeMatch(sig_func, slot_func))
    {
        error = "signal slot not match " + sig_func->fullName() + "," + slot_func->delcare();
        return false;
    }

    JZFunctionPointer sig_ptr;
    sig_ptr.functionName = sig;
    JZFunctionPointer slot_ptr;
    slot_ptr.functionName = slot;

    int send_id = c->paramId(m_id, paramIn(0));
    int recv_id = c->paramId(m_id, paramIn(2));
    QList<JZNodeIRParam> in, out;
    in << irId(send_id) << irLiteral(QVariant::fromValue(sig_ptr)) << irId(recv_id) << irLiteral(QVariant::fromValue(slot_ptr));
    c->addCall("connect", in, out);

    return true;
}

//JZNodeSignalDisconnect
JZNodeSignalDisconnect::JZNodeSignalDisconnect()
{
}

JZNodeSignalDisconnect::~JZNodeSignalDisconnect()
{
}

bool JZNodeSignalDisconnect::compiler(JZNodeCompiler* c, QString& error)
{
    return false;
}

// JZNodeEvent
JZNodeEvent::JZNodeEvent()
{
    m_type = Node_none;

    addFlowOut();
}

JZNodeEvent::~JZNodeEvent()
{

}

const JZNodeObjectDefine* JZNodeEvent::classMeta()
{
    auto class_item = m_file->getClassItem();
    if (!class_item)
        return nullptr;

    return environment()->meta(class_item->className());
}

//JZNodeFunctionStart
JZNodeFunctionStart::JZNodeFunctionStart()
{
    m_name = "Start";
    m_type = Node_functionStart;
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
    c->addNodeEnter(m_id);
    c->addFlowOutput(m_id);
    return true;
}

///JZNodeShowEvent
JZFunctionDefine JZNodeShowEvent::function()
{
    return JZFunctionDefine();
}
bool JZNodeShowEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return false;
}

//JZNodeCloseEvent
JZFunctionDefine JZNodeCloseEvent::function()
{
    return JZFunctionDefine();
}

bool JZNodeCloseEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return false;
}

//JZNodeResizeEvent
JZFunctionDefine JZNodeResizeEvent::function()
{
    return JZFunctionDefine();
}

bool JZNodeResizeEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return false;
}

//JZNodePaintEvent
JZFunctionDefine JZNodePaintEvent::function()
{
    return JZFunctionDefine();
}
bool JZNodePaintEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return false;
}

//JZNodeMousePressEvent
JZFunctionDefine JZNodeMousePressEvent::function()
{
    return JZFunctionDefine();
}
bool JZNodeMousePressEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return false;
}

//JZNodeMouseReleaseEvent
JZFunctionDefine JZNodeMouseReleaseEvent::function()
{
    return JZFunctionDefine();
}
bool JZNodeMouseReleaseEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return false;
}

//JZNodeMouseMoveEvent
JZFunctionDefine JZNodeMouseMoveEvent::function()
{
    return JZFunctionDefine();
}

bool JZNodeMouseMoveEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return false;
}

//JZNodeKeyPressEvent
JZFunctionDefine JZNodeKeyPressEvent::function()
{
    return JZFunctionDefine();
}

bool JZNodeKeyPressEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return false;
}


//JZNodeButtonClickedEvent
JZNodeButtonClickedEvent::JZNodeButtonClickedEvent()
{
    m_type = Node_buttonClikedEvnet;
}

JZNodeButtonClickedEvent::~JZNodeButtonClickedEvent()
{
}

void JZNodeButtonClickedEvent::setObject(QString name)
{
    m_object = name;
}

QString JZNodeButtonClickedEvent::object()
{
    return m_object;
}

JZFunctionDefine JZNodeButtonClickedEvent::function()
{
    return JZFunctionDefine();
}

bool JZNodeButtonClickedEvent::compiler(JZNodeCompiler* compiler, QString& error)
{
    return false;
}

//JZNodeSignalEvent
JZFunctionDefine JZNodeSignalEvent::function()
{
    auto meta = classMeta();
    if (!meta)
        return JZFunctionDefine();

    QString file_name = m_file->name();
    JZFunctionDefine define = meta->initMemberFunction(m_name + "_" + QString::number(m_id) + "_" + file_name);
    define.paramOut = functionParamOut();
    return define;    
}

QList<JZParamDefine> JZNodeSignalEvent::functionParamOut()
{
    return QList<JZParamDefine>();
}

bool JZNodeSignalEvent::compilerSignal(JZNodeCompiler* c,const QJsonObject &object, QString& error)
{
    auto meta = classMeta();
    if (!meta || !meta->isInherits(Type_object))
    {
        error = "only support define in object";
        return false;
    }

    c->addFunctionAlloc(function());
    c->addNodeEnter(m_id);

    QString function_name = function().fullName();
    QJsonObject obj = object;
    obj["function"] = function_name;    

    QByteArray buffer = QJsonDocument(obj).toJson();
    c->addConstructor(m_constructor,buffer);
    return true;
}

//JZNodeTimerEvent
JZNodeTimerEvent::JZNodeTimerEvent()
{
    m_timeout = 1000;
    m_name = "timerEvent";

    m_constructor.function = "JZTimerEventConnect";
}

JZNodeTimerEvent::~JZNodeTimerEvent()
{
}

void JZNodeTimerEvent::setTimeOut(int ms)
{
    m_timeout = ms;
}

int JZNodeTimerEvent::timeOut()
{
    return m_timeout;
}

bool JZNodeTimerEvent::compiler(JZNodeCompiler* c, QString& error)
{    
    QJsonObject obj;
    obj["timeout"] = m_timeout;
    return compilerSignal(c,obj,error);
}

void JZNodeTimerEvent::saveToStream(QDataStream &s) const
{
    JZNodeSignalEvent::saveToStream(s);
    s << m_timeout;
}

void JZNodeTimerEvent::loadFromStream(QDataStream &s)
{
    JZNodeSignalEvent::loadFromStream(s);
    s >> m_timeout;
}

void JZTimerEventConnect(QObject *object,const QByteArray &buffer)
{
    QJsonObject obj = JZNodeJson::formBuffer(buffer);
    QString slot_function = obj["function"].toString();
    int ms = obj["timeout"].toInt();

    QTimer *timer = new QTimer(object);
    timer->connect(timer,&QTimer::timeout,object,[object,slot_function]
    {
        JZNodeObject *jzobj = qobjectToJZObject(object);
        JZNodeObjectHolder self(jzobj, false);
        QVariantList in,out;
        in << QVariant::fromValue(self.toPointer());
        JZScriptInvoke(slot_function,in,out);
    });
    timer->start(ms);
}

//JZNodeEventFunctionInit
void JZNodeEventFunctionInit(JZScriptEnvironment *env)
{
    auto func_inst = env->functionManager();
    func_inst->registCFunction("JZTimerEventConnect",true,jzbind::createFuncion(JZTimerEventConnect));
}