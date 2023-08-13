#include "JZNodeUtils.h"

QString makeLink(QString filename, QString function, int nodeId)
{
    QString link = "link:" + filename + "(" + function + ",id=" + QString::number(nodeId) + ")";
    return link;
}