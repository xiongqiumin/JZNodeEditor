#include "JZNodeUtils.h"
#include "JZNodeView.h"

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
    auto item_list = project->itemList("./", ProjectItem_any);
    for (int i = 0; i < item_list.size(); i++)
    {
        int item_type = item_list[i]->itemType();
        if (item_type == ProjectItem_scriptFlow || item_type == ProjectItem_scriptFunction
            || item_type == ProjectItem_scriptParamBinding)
        {
            JZNodeView *view = new JZNodeView();
            JZScriptFile *file = (JZScriptFile *)item_list[i];
            view->setFile(file);
            view->updateNodeLayout();
            view->save();
            delete view;
        }
    }
    project->saveAllItem();
}