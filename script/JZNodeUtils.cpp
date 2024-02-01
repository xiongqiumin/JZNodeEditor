#include "JZNodeUtils.h"

QString makeLink(QString filename, QString function, int nodeId)
{
    QString link;
    if (!filename.isEmpty())
        link = "link:" + function + "(" + filename + ",id=" + QString::number(nodeId) + ")";
    else
        link = function;
    return link;
}