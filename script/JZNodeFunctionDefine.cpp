#include "JZNodeFunctionDefine.h"

//FunctionDefine
FunctionDefine::FunctionDefine()
{
    isCFunction = false;
    addr = 0;
    pointer = nullptr;
    functionType = Func_none;
}