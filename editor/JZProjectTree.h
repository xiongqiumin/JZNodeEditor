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
    void clear();
    void init();

protected slots:
    void onContextMenu(QPoint pos);

    void onItemChanged(QTreeWidgetItem *item);
    void onItemClicked(QTreeWidgetItem *item);
    void onItemDoubleClicked(QTreeWidgetItem *item);
    void onItemRename();
    void onCurrentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);    

signals:
    void sigFileOpened(QString file);  

protected:    
    void newFunctionItem();
    void newClassItem();
    void addItem(QTreeWidgetItem *view_item,JZProjectItem *item);
    JZProjectItem *getItem(QTreeWidgetItem *view_item);
    void cancelEdit();

    JZProject *m_project;   
    QTreeWidget *m_tree; 

    QTreeWidgetItem *m_editItem;
    JZProjectItem *m_editProjectItem;    
};
















#endif
