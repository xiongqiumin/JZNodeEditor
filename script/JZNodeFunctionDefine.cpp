#include <QDebug>
#include "JZNodeFunctionDefine.h"
#include "JZNodeObject.h"
#include "JZRegExpHelp.h"

//JZParamDefine
JZParamDefine::JZParamDefine()
{    
}

JZParamDefine::JZParamDefine(QString param_name, QString dataType, const QString &v)
{
    this->name = param_name;
    this->type = dataType;
    this->value = v;
}

QDataStream &operator<<(QDataStream &s, const JZParamDefine &param)
{
    s << param.name;
    s << param.type;
    s << param.value;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZParamDefine &param)
{
    s >> param.name;
    s >> param.type;
    s >> param.value;
    return s;
}

//JZNodeParamBind
JZNodeParamBind::JZNodeParamBind()
{
    dir = UiToData;
}

QDataStream &operator<<(QDataStream &s, const JZNodeParamBind &param)
{
    s << param.widget;
    s << param.widgetProp;
    s << param.variable;
    s << param.dir;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZNodeParamBind &param)
{
    s >> param.widget;
    s >> param.widgetProp;
    s >> param.variable;
    s >> param.dir;
    return s;
}

//CFunction
CFunction::CFunction()
{

}

CFunction::~CFunction()
{

}

//BuiltInFunction
BuiltInFunction::BuiltInFunction()
{    
}

BuiltInFunction::~BuiltInFunction()
{
}

//JZFunctionDefine
JZFunctionDefine::JZFunctionDefine()
{
    isCFunction = false;
    isFlowFunction = true;    
    isVirtualFunction = false;
    isProtected = false;  
    isSlot = false;
    isSingle = false;
}

bool JZFunctionDefine::isNull() const
{
    return name.isEmpty();
}

void JZFunctionDefine::setFullName(const QString &full_name)
{
    QString class_name,member_name;
    JZRegExpHelp::splitDefine(full_name,class_name,member_name);
    
    if(!class_name.isEmpty())
    {
        className = class_name;
        name = member_name;
    }
    else
    {
        className.clear();
        name = member_name;
    }
}

QString JZFunctionDefine::fullName() const
{
    QString result = name;
    if (!className.isEmpty())
        result = className + "::" + result;
        
    return result;
}

QString JZFunctionDefine::delcare() const
{
    QString text;
    QStringList in_list,out_list;
    for(int i = 0; i < paramIn.size(); i++)
        in_list << paramIn[i].type;
    for(int i = 0; i < paramOut.size(); i++)
        out_list << paramOut[i].type;
    if(out_list.size() == 0)
        text = "void";
    else
        text = out_list.join(",");

    text += " " + fullName() + "(" + in_list.join(",") + ")";
    return text;
}

bool JZFunctionDefine::isMemberFunction() const
{
    return (paramIn.size() > 0 && paramIn[0].name == "this");
}

bool JZFunctionDefine::isVariadicFunction() const
{
    return (paramIn.size() > 0 && paramIn.back().type == JZNodeType::typeName(Type_args));
}

void JZFunctionDefine::setDefaultValue(int index, QString text)
{
    paramIn[index].value = text;
}

void JZFunctionDefine::setDefaultValue(int index, QStringList values)
{
    for (int i = 0; i < values.size(); i++)
    {
        paramIn[i + index].value = values[i];
    }
}

QDataStream &operator<<(QDataStream &s, const JZFunctionDefine &param)
{    
    s << param.name;
    s << param.className;
    s << param.isCFunction;
    s << param.isFlowFunction;
    s << param.isVirtualFunction;
    s << param.isProtected;
    s << param.isSingle;
    s << param.isSlot;
    s << param.paramIn;
    s << param.paramOut;         
    return s;
}

QDataStream &operator>>(QDataStream &s, JZFunctionDefine &param)
{
    s >> param.name;
    s >> param.className;
    s >> param.isCFunction;
    s >> param.isFlowFunction;
    s >> param.isVirtualFunction;
    s >> param.isProtected;
    s >> param.isSingle;
    s >> param.isSlot;
    s >> param.paramIn;
    s >> param.paramOut;          
    return s;
}

//CSignal
CSignal::CSignal()
{    
}

CSignal::~CSignal()
{

}

void CSignal::disconnect(JZNodeObject* sender, QString slot)
{
    removeConnect(sender, nullptr, slot);
}

void CSignal::disconnect(JZNodeObject* sender, JZNodeObject* recv, QString slot)
{
    removeConnect(sender, recv, slot);
}

void CSignal::addConnect(JZNodeObject* obj, JZNodeObject* recv, QString slot, QMetaObject::Connection conn)
{
}

void CSignal::removeConnect(JZNodeObject* obj, JZNodeObject* recv, QString slot)
{
    for (int i = 0; i < m_connects.size(); i++)
    {
        auto& info = m_connects[i];
        if (obj == info.send && recv == info.recv && slot == info.slot)
        {
            QObject* qobj = obj->manager()->objectCast<QObject>(obj);
            qobj->disconnect(info.conn);
            return;
        }
    }
}

//JZSignalDefine
JZSignalDefine::JZSignalDefine()
{
    csignal = nullptr;
}

QString JZSignalDefine::fullName() const
{
    return className + "::" + name;
}

bool JZSignalDefine::isCSignal() const
{
    return (csignal != nullptr);
}

QDataStream &operator<<(QDataStream &s, const JZSignalDefine &param)
{
    Q_ASSERT(!param.isCSignal());
    s << param.name;
    s << param.className;
    s << param.paramOut;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZSignalDefine &param)
{
    s >> param.name;
    s >> param.className;
    s >> param.paramOut;
    return s;
}

//JZSlotFunctionDefine
JZFunctionDefine JZSlotFunctionDefine(QString sender, const JZSignalDefine* signal)
{
    JZFunctionDefine func_def;
    func_def.name = "on_" + sender + "_" + signal->name;
    for (int i = 0; i < signal->paramOut.size(); i++)
    {
        func_def.paramIn.push_back(signal->paramOut[i]);
    }
    return func_def;
}

//JZParam
JZParam::JZParam()
{
    dataType = Type_none;
}

JZParam::JZParam(const QString &param_name, int type)
{
    this->name = param_name;
    this->dataType = type;
}

QDataStream &operator<<(QDataStream &s, const JZParam &param)
{
    s << param.name;
    s << param.dataType;    
    return s;
}

QDataStream &operator>>(QDataStream &s, JZParam &param)
{
    s >> param.name;
    s >> param.dataType;    
    return s;
}

//JZFunction
JZFunction::JZFunction()
{
    addr = -1;
    addrEnd = -1;    
}

JZFunction::~JZFunction()
{

}

QString JZFunction::name() const
{
    return define.name;
}

QString JZFunction::className() const
{
    return define.className;
}

QString JZFunction::fullName() const
{
    return define.fullName();
}

bool JZFunction::isCFunction() const
{
    return define.isCFunction;
}

bool JZFunction::isMemberFunction() const
{
    return define.isMemberFunction();
}

bool JZFunction::isFlowFunction() const
{
    return define.isFlowFunction;
}

bool JZFunction::isVirtualFunction() const
{
    return define.isVirtualFunction;
}

QDataStream &operator<<(QDataStream &s, const JZFunction &param)
{
    s << param.define;
    s << param.path;
    s << param.addr;
    s << param.addrEnd;
    return s;
}

QDataStream &operator >> (QDataStream &s, JZFunction &param)
{
    s >> param.define;
    s >> param.path;
    s >> param.addr;
    s >> param.addrEnd;
    return s;
}
