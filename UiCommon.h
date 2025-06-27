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
    static QWidget *createVBox(QList<QWidget*> list);
    static QWidget *createHBox(QList<QWidget*> list);

    static bool treeFilter(QTreeWidgetItem *tree, QString name);    
    static int treeIndexOf(QTreeWidgetItem *root,const QString &name);
    static void treeSortChilds(QTreeWidgetItem *node);
    static void treeSortChilds(QTreeWidgetItem *node,const std::function<bool(QTreeWidgetItem*,QTreeWidgetItem*)> &cmp);
    static void treeClearChildren(QTreeWidgetItem *root);    //删除子节点
    static QList<QTreeWidgetItem*> treeFindItem(QTreeWidgetItem *root,int column,int role,QVariant value);
    static void treeVisit(QTreeWidgetItem *root,const std::function<void(QTreeWidgetItem*)> &visitor); 
};

#endif