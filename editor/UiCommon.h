#ifndef UI_COMMON_H_
#define UI_COMMON_H_

#if defined(_MSC_VER) && _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMessageBox>
#include <QTreeWidget>
#include <QComboBox>
#include <functional>

class QTreeWidget;
class UiHelper
{
public:
    static bool treeFilter(QTreeWidgetItem *tree, QString name);
    static void treeUpdate(QTreeWidgetItem *root,const QStringList &names, std::function<QTreeWidgetItem*(int)> func); //返回新增的item
    static void clearTreeItem(QTreeWidgetItem *root);
    static void updateEnumBox(QComboBox *box,int dataType,int value = 0xfafa);
};


#endif