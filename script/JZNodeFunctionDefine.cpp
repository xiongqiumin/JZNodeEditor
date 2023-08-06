#include "JZNodeFunctionDefine.h"
#include "JZEvent.h"

//CFunction
CFunction::CFunction()
{

}

CFunction::~CFunction()
{

}

//FunctionDefine
FunctionDefine::FunctionDefine()
{
    isCFunction = false;
    isFlowFunction = false;      
    cfunc = nullptr;
}

QString FunctionDefine::fullName() const
{
    QString result = name;
    if (!className.isEmpty())
        result = className + "." + result;
    return result;
}

QDataStream &operator<<(QDataStream &s, const FunctionDefine &param)
{
    Q_ASSERT(!param.isCFunction);

    s << param.name;
    s << param.className;

    s << param.isFlowFunction;
    s << param.paramIn;
    s << param.paramOut;     
    return s;
}

QDataStream &operator>>(QDataStream &s, FunctionDefine &param)
{
    s >> param.name;
    s >> param.className;

    s >> param.isFlowFunction;     
    s >> param.paramIn;
    s >> param.paramOut;      
    return s;
}

//EventDefine
EventDefine::EventDefine()
{
    eventType = Event_none;
}

QDataStream &operator<<(QDataStream &s, const EventDefine &param)
{
    s << param.eventType;
    s << param.name;
    s << param.paramOut;
    return s;
}
QDataStream &operator >> (QDataStream &s, EventDefine &param)
{
    s >> param.eventType;
    s >> param.name;
    s >> param.paramOut;
    return s;
}
