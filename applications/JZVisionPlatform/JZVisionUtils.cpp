#include "JZVisionUtils.h"

QIcon JZVisionUtils::icon(QString name)
{
    QString icon_path = ":/JZMonitorSystem/Resources/icons/" + name;
    return QIcon(icon_path);
}