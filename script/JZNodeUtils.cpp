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

QString makeLink(QString tips, QString path, QString args)
{
    QString href = path + "?" + args;
    QString link = "<link href=" + href + ">" + tips + "</link>";
    return link;
}

JZUrl fromQUrl(QUrl url)
{
    JZUrl ret;
    ret.path = url.path();

    if (url.hasQuery())
    {
        QStringList query_list = url.query().split("&");
        for (int i = 0; i < query_list.size(); i++)
        {
            auto pairs = query_list[i].split("=");
            ret.args[pairs[0]] = pairs[1];
        }        
    }
    return ret;
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

QString JZNodeUtils::className(QString name)
{
    MemberInfo ret = jzSplitMember(name);
    return ret.className;
}