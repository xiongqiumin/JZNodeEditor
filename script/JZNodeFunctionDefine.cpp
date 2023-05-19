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
    addr = 0;
    script = nullptr;       
    cfunc = nullptr;
}
