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

void UiHelper::treeClearChildren(QTreeWidgetItem *root)
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

void UiHelper::treeSortChilds(QTreeWidgetItem *node)
{
    treeSortChilds(node, [](QTreeWidgetItem *item1, QTreeWidgetItem *item2)->bool {
        return item1->text(0) < item2->text(0);
    });
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

    for (int i = count - 1; i >= 0; i--)
        root->takeChild(i);
    
    Q_ASSERT(root->childCount() == 0);
    for (int i = 0; i < sort_list.size(); i++)
        root->addChild(sort_list[i]);   
}