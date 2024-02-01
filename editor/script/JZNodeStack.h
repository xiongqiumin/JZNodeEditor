#ifndef JZNODE_STACK_H_
#define JZNODE_STACK_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTableWidget>
#include "JZNodeEngine.h"

class JZNodeStack : public QWidget
{
    Q_OBJECT

public:
    JZNodeStack(QWidget *parent = nullptr);
    ~JZNodeStack();
    
    void setRuntime(JZNodeRuntimeInfo info);
    void setRunning(bool isRun);
    void enterPressed();

signals:
    void sigStackChanged(int level);

protected slots:
    void onItemDoubleClicked(QTableWidgetItem *item);

protected:       
    void updateStatus();
    void stackChanged(int level);

    bool m_running;
    int m_status;

    QTableWidget *m_table;
};






#endif
