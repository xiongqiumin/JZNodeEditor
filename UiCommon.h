#ifndef UI_COMMON_H_
#define UI_COMMON_H_

#include <QMessageBox>
#include <QTreeWidget>
#include <QComboBox>
#include <functional>


class QTreeWidget;

class UiHelper
{
public:
    static bool treeFilter(QTreeWidgetItem *tree, QString name);    
    static int treeIndexOf(QTreeWidgetItem *root,const QString &name);
    static void treeSortChilds(QTreeWidgetItem *node);
    static void treeSortChilds(QTreeWidgetItem *node,std::function<bool(QTreeWidgetItem*,QTreeWidgetItem*)> cmp);
    static void treeClearChildren(QTreeWidgetItem *root);    //删除子节点
};

#endif