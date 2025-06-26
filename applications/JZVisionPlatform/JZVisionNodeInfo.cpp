#include "JZVisionNodeInfo.h"

JZVisionNodeInfo::JZVisionNodeInfo()
{
    node = nullptr;
    hasImage = false;
}

//JZVisionRuntimeResult
JZVisionRuntimeResult::JZVisionRuntimeResult()
{

}

void JZVisionRuntimeResult::clear()
{
    nodeResult.clear();
}