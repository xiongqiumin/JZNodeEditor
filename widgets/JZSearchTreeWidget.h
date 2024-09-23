#ifndef JZ_SEARCH_TREE_WIDGET_H_
#define JZ_SEARCH_TREE_WIDGET_H_

#include <QTreeWidget>
#include <QLineEdit>
#include "JZNodeCoreDefine.h"

class JZCORE_EXPORT JZSearchTreeWidget: public QWidget
{
    Q_OBJECT

public:
    JZSearchTreeWidget();
    ~JZSearchTreeWidget();

    QTreeWidget *tree();

protected slots:
    void onSearch();

protected:
    bool filterItem(QTreeWidgetItem *item, QString name);

    QLineEdit *m_lineSearch;
    QTreeWidget *m_tree;
};

#endif // ! JZ_SEARCH_TREE_WIDGET_H_
