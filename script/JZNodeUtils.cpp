#include "JZNodeUtils.h"
#include "JZNodeView.h"
#include <QDateTime>
#include <QDebug>

MemberInfo jzSplitMember(QString fullName)
{
    MemberInfo info;
    QStringList list = fullName.split(".");
    if (list.size() > 1)
        info.className = list[0];
    
    info.name = list.back();
    return info;
}

QString makeLink(QString tips, QString filename, int nodeId)
{
    QString href = filename + "?id=" + QString::number(nodeId);
    QString link;
    if (!filename.isEmpty())
        link = tips + "<link href=" + href + ">(" + filename + ",id=" + QString::number(nodeId) + ")</link>";
    else
        link = tips;
    return link;
}

void projectUpdateLayout(JZProject *project)
{
    auto item_list = project->itemList("./", ProjectItem_any);
    for (int i = 0; i < item_list.size(); i++)
    {
        int item_type = item_list[i]->itemType();
        if (item_type == ProjectItem_scriptFunction
            || item_type == ProjectItem_scriptParamBinding)
        {
            JZScriptItem *item = (JZScriptItem *)item_list[i];
            jzScriptItemUpdateLayout(item);
        }
    }    
}

void jzScriptItemUpdateLayout(JZScriptItem *item)
{
    JZNodeView *view = new JZNodeView();
    view->setFile(item);
    view->updateNodeLayout();
    view->save();
    delete view;
}

void jzScriptItemDump(JZScriptItem *item,QString file)
{
    JZNodeView *view = new JZNodeView();
    view->setFile(item);

    QImage image(800,600,QImage::Format_RGB32);
    image.fill(Qt::white);
    view->resize(image.size());
    view->fitNodeView();

    QPainter pt(&image);
    view->render(&pt);
    delete view;

    image.save(file);
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