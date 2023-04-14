#ifndef JZPROJECT_TREE_H_
#define JZPROJECT_TREE_H_

#include <QWidget>
#include "JZProject.h"
#include <QTreeWidget>

class JZProjectTree : public QWidget
{
    Q_OBJECT

public:
    JZProjectTree();
    ~JZProjectTree();

    void setProject(JZProject *project);

protected slots:
    void onContextMenu(QPoint pos);
    void onItemDoubleClicked(QTreeWidgetItem *item);

signals:
    void sigFileOpened(QString file);

protected:
    void init();
    void addItem(QTreeWidgetItem *item,JZProjectItem *item);

    JZProject *m_project;   
    QTreeWidget *m_tree; 
};
















#endif
