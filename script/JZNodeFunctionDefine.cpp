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
    addr = -1;
}

bool FunctionDefine::isNull() const
{
    return name.isEmpty();
}

QString FunctionDefine::fullName() const
{
    QString result = name;
    if (!className.isEmpty())
        result = className + "." + result;
    return result;
}

void FunctionDefine::updateCFunctionParam()
{
    Q_ASSERT(cfunc);
    paramIn.clear();
    paramOut.clear();

    for (int i = 0; i < cfunc->args.size(); i++)
    {
        JZParamDefine prop;
        prop.name = "input" + QString::number(i);
        prop.dataType = JZNodeType::typeidToType(cfunc->args[i]);
        Q_ASSERT(prop.dataType != Type_none);

        paramIn.push_back(prop);
    }
    if (cfunc->result != typeid(void).name())
    {
        JZParamDefine prop;
        prop.name = "output";
        prop.dataType = JZNodeType::typeidToType(cfunc->result);
        Q_ASSERT(prop.dataType != Type_none);

        paramOut.push_back(prop);
    }
}

bool FunctionDefine::isMemberFunction() const
{
    return !className.isEmpty();
}

QDataStream &operator<<(QDataStream &s, const FunctionDefine &param)
{
    Q_ASSERT(!param.isCFunction);

    s << param.name;
    s << param.className;

    s << param.isFlowFunction;
    s << param.paramIn;
    s << param.paramOut;     

    s << param.file;
    s << param.addr;
    return s;
}

QDataStream &operator>>(QDataStream &s, FunctionDefine &param)
{
    s >> param.name;
    s >> param.className;

    s >> param.isFlowFunction;     
    s >> param.paramIn;
    s >> param.paramOut;      

    s >> param.file;
    s >> param.addr;
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
    return s;
}
QDataStream &operator >> (QDataStream &s, EventDefine &param)
{
    s >> param.eventType;
    s >> param.name;
    return s;
}
