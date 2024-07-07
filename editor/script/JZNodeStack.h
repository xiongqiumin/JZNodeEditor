#ifndef JZNODE_STACK_H_
#define JZNODE_STACK_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTableWidget>
#include "JZNodeEngine.h"
#include "JZProcess.h"

class JZNodeStack : public QWidget
{
    Q_OBJECT

public:
    JZNodeStack(QWidget *parent = nullptr);
    ~JZNodeStack();
    
    void setRuntime(const JZNodeRuntimeInfo &info);
    void setStackIndex(int stack);
    int stackIndex();

    void setRunningMode(ProcessStatus flag);    

signals:
    void sigStackChanged(int level);

protected slots:
    void onItemDoubleClicked(QTableWidgetItem *item);

protected:       
    virtual void keyPressEvent(QKeyEvent *e) override;

    void updateStatus();
    void stackChanged(int level);
    
    ProcessStatus m_status;
    int m_stackIndex;

    QTableWidget *m_table;
};






#endif
