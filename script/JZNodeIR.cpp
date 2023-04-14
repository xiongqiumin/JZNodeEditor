#include "JZNodeIR.h"

JZNodeIR::JZNodeIR()
{
    type = OP_none;
    source = -1;
}

JZNodeIR::JZNodeIR(int t)
{
    type = t;
    source = -1;    
}