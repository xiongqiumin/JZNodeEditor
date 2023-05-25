#include "JZNodeBind.h"

namespace jzbind
{

template<>
QVariant getValue<QVariant>(QVariant v,std::false_type)
{
    return v;
}

template<>
QVariant getReturn(QVariant value)
{
    return value;
}

}


