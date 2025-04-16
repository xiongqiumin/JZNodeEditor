#include <QTreeWidget>
#include <QComboBox>

#include "UiCommon.h"
#include "JZNodeObject.h"

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

int UiHelper::treeIndexOf(QTreeWidgetItem *node,const QString &name)
{
    for (int i = 0; i < node->childCount(); i++)
    {
        auto sub = node->child(i);
        if (sub->text(0) == name)
            return i;
    }
    return -1;
}

void UiHelper::treeSortChilds(QTreeWidgetItem *root,std::function<bool(QTreeWidgetItem*,QTreeWidgetItem*)> cmp)
{
    int count = root->childCount();
    QList<QTreeWidgetItem*> sort_list;
    for (int i = count - 1; i >= 0; i--)
    {
        sort_list << root->child(i);
    }
    std::sort(sort_list.begin(), sort_list.end(), cmp);

    bool need_sort = false;
    for (int i = 0; i < count; i++)
    {
        if (sort_list[i] != root->child(i))
        {
            need_sort = true;
            break;
        }
    }
    if (!need_sort)
        return;

    for (int i = 0; i < count; i++)
        root->takeChild(i);

    for (int i = 0; i < sort_list.size(); i++)
        root->addChild(sort_list[i]);   
}