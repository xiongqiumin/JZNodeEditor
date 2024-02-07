#ifndef JZPROJECT_TREE_H_
#define JZPROJECT_TREE_H_

#include <QWidget>
#include "JZProject.h"
#include <QTreeWidget>
#include "UiCommon.h"

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
    void sigFileRemoved(QString file);

protected:    
    void addItem(QTreeWidgetItem *parent, JZProjectItem *item);
    void setItem(QTreeWidgetItem *view_item,JZProjectItem *item);
    QTreeWidgetItem *getItem(QString path);
    JZProjectItem *getFile(QTreeWidgetItem *view_item);

    QString getClass(JZProjectItem *item);
    void cancelEdit();
    QString filepath(QTreeWidgetItem *item);
    void renameItem(QTreeWidgetItem *item);    

    JZProject *m_project;   
    QTreeWidget *m_tree; 

    QTreeWidgetItem *m_editItem;    
};
















#endif
