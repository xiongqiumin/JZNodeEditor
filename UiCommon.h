#ifndef UI_COMMON_H_
#define UI_COMMON_H_

#include <QMessageBox>
#include <QTreeWidget>
#include <QComboBox>
#include <functional>


class QTreeWidget;

class TreeDiffResult
{
public:
    enum{
        Add,
        Remove,        
    };

    QString name;
    int type;
};

class UiHelper
{
public:
    static bool treeFilter(QTreeWidgetItem *tree, QString name);
    static QList<TreeDiffResult> treeDiff(QTreeWidgetItem *root,const QStringList &list);
    static int treeIndexOf(QTreeWidgetItem *root,const QString &name);
    static void clearTreeItem(QTreeWidgetItem *root);    
};

#endif