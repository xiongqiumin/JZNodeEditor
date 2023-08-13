#ifndef JZNODE_BREAKPOINT_WIDGET_H_
#define JZNODE_BREAKPOINT_WIDGET_H_

#include <QTableWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTreeWidget>
#include "JZProject.h"

class MainWindow;
class JZNodeBreakPoint : public QWidget
{
    Q_OBJECT

public:
    JZNodeBreakPoint(QWidget *parent = nullptr);
    ~JZNodeBreakPoint();
    
    void setMainWindow(MainWindow *w);
    void updateBreakPoint(JZProject *project);
    void clear();

signals:    
    void sigBreakPointClicked();    

protected slots:       
    void onItemDoubleClicked();

protected:       
    void updateStatus();    
    
    QTableWidget *m_table;
    MainWindow *m_window;
};






#endif
