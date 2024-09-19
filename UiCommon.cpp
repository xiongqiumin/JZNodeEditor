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

QList<TreeDiffResult> UiHelper::treeDiff(QTreeWidgetItem *root, const QStringList &names)
{    
    QList<TreeDiffResult> list;
    for (int i = 0; i < root->childCount(); i++)
    {
        if (!names.contains(root->child(i)->text(0)))
        {
            TreeDiffResult ret;
            ret.name = root->child(i)->text(0);
            ret.type = TreeDiffResult::Remove;
            list << ret;
        }
    }
    
    for (int i = 0; i < names.count(); i++)
    {
        QTreeWidgetItem *item = nullptr;
        int cur_index = treeIndexOf(root, names[i]);
        if (cur_index == -1)
        {            
            TreeDiffResult ret;
            ret.name = names[i];
            ret.type = TreeDiffResult::Add;
            list << ret;
        }
    }
    return list;
}