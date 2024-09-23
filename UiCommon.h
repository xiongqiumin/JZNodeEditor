#ifndef UI_COMMON_H_
#define UI_COMMON_H_

#if defined(_MSC_VER) && _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMessageBox>
#include <QTreeWidget>
#include <QComboBox>
#include <functional>
#include "JZNodeCoreDefine.h"

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