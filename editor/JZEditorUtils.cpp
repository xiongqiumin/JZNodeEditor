#include "JZEditorUtils.h"
#include "JZNodeView.h"

void JZEditorUtils::projectUpdateLayout(JZProject *project)
{
    auto item_list = project->itemList("./", ProjectItem_any);
    for (int i = 0; i < item_list.size(); i++)
    {
        int item_type = item_list[i]->itemType();
        if (item_type == ProjectItem_scriptFunction)
        {
            JZScriptItem *item = (JZScriptItem *)item_list[i];
            scriptItemUpdateLayout(item);
        }
    }    
}

void JZEditorUtils::scriptItemUpdateLayout(JZScriptItem *item)
{
    JZNodeView *view = new JZNodeView();
    view->setFile(item);
    view->updateNodeLayout();
    view->save();
    delete view;
}

void JZEditorUtils::scriptItemDump(JZScriptItem *item,QString file)
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