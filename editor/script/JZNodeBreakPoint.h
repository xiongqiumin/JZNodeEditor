#ifndef JZNODE_BREAKPOINT_WIDGET_H_
#define JZNODE_BREAKPOINT_WIDGET_H_

#include <QTableWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTreeWidget>
#include "JZProject.h"

class JZNodeBreakPoint : public QWidget
{
    Q_OBJECT

public:
    JZNodeBreakPoint(QWidget *parent = nullptr);
    ~JZNodeBreakPoint();
    
    void setProject(JZProject *project);
    void updateBreakPoint();
    void clear();

signals:    
    void sigBreakPointClicked(QString file,int id);        

protected slots:       
    void onItemDoubleClicked();
    void onContexMenu(QPoint pt);

protected:               
    QTableWidget *m_table;
    JZProject *m_project;
};






#endif
