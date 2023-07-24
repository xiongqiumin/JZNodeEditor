#include "JZNodeFunctionDefine.h"

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
    addr = 0;
    script = nullptr;       
    cfunc = nullptr;
}

QDataStream &operator<<(QDataStream &s, const FunctionDefine &param)
{
    Q_ASSERT(!param.isCFunction);

    s << param.isCFunction;
    s << param.name;
    s << param.paramIn;
    s << param.paramOut;
    s << param.addr;
    s << param.script;
    return s;
}

QDataStream &operator>>(QDataStream &s, FunctionDefine &param)
{
    s >> param.isCFunction;
    s >> param.name;
    s >> param.paramIn;
    s >> param.paramOut;
    s >> param.addr;
    s >> param.script;
    return s;
}
