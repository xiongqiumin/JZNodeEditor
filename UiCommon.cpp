#include <QTreeWidget>
#include "UiCommon.h"

bool UiHelper::treeFilter(QTreeWidgetItem *item, QString name)
{
    bool show = false;
    int count = item->childCount();
    if (count == 0)
    {
        show = item->text(0).contains(name);
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            if (treeFilter(item->child(i), name))
                show = true;
        }
    }
    item->setHidden(!show);
    return show;
}

void UiHelper::clearTreeItem(QTreeWidgetItem *root)
{
    while (root->childCount() > 0)
        delete root->takeChild(0);
}

void UiHelper::treeUpdate(QTreeWidgetItem *root, const QStringList &names, std::function<QTreeWidgetItem*(int)> func)
{    
    auto indexOfItem = [](QTreeWidgetItem *root,const QString &name)->int{
        for (int i = 0; i < root->childCount(); i++)
        {
            auto sub = root->child(i);
            if (sub->text(0) == name)
                return i;
        }
        return -1;
    };

    for (int i = root->childCount() - 1; i >= 0; i--)
    {
        if (!names.contains(root->child(0)->text(0)))
            delete root->takeChild(i);
    }
    
    for (int i = 0; i < names.count(); i++)
    {
        QTreeWidgetItem *item = nullptr;
        int cur_index = indexOfItem(root, names[i]);
        if (cur_index >= 0)
        {
            item = root->child(cur_index);
            if (cur_index != i)
            {
                root->takeChild(cur_index);
                root->insertChild(i, item);
            }            
        }
        else
        {            
            item = func(i);            
            root->addChild(item);
        }
    }    
}