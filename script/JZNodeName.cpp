#include "JZNodeName.h"

bool JZNodeName::isVaildName(QString name)
{
    return true;
}

QString JZNodeName::memberName(QString name)
{
    int index = name.lastIndexOf(".");
    if (index == -1)
        return name;
    else
        return name.mid(index+1);
}