#include "JZNodeUtils.h"
#include "JZNodeView.h"
#include <QDateTime>
#include <QDebug>

QString makeLink(QString tips, QString filename, int nodeId)
{
    QString link;
    if (!filename.isEmpty())
        link = "link:" + tips + "(" + filename + ",id=" + QString::number(nodeId) + ")";
    else
        link = tips;
    return link;
}

void projectUpdateLayout(JZProject *project)
{
    TimerRecord r("projectUpdateLayout");

    auto item_list = project->itemList("./", ProjectItem_any);
    for (int i = 0; i < item_list.size(); i++)
    {
        int item_type = item_list[i]->itemType();
        if (item_type == ProjectItem_scriptFlow || item_type == ProjectItem_scriptFunction
            || item_type == ProjectItem_scriptParamBinding)
        {
            JZNodeView *view = new JZNodeView();
            JZScriptItem *file = (JZScriptItem *)item_list[i];
            view->setFile(file);
            view->updateNodeLayout();
            view->save();
            delete view;
        }
    }    
}

TimerRecord::TimerRecord(QString name)
{
    m_name = name;
    m_time = QDateTime::currentMSecsSinceEpoch();
}

TimerRecord::~TimerRecord()
{
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    qDebug().noquote() << m_name + ": " + QString::number(time - m_time) + "ms";
}