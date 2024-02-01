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
    void setStackIndex(int stack);
    int stackIndex();

    void setRunning(bool isRun);
    void enterPressed();

signals:
    void sigStackChanged(int level);

protected slots:
    void onItemDoubleClicked(QTableWidgetItem *item);

protected:       
    virtual void keyPressEvent(QKeyEvent *e) override;

    void updateStatus();
    void stackChanged(int level);

    bool m_running;
    int m_status;
    int m_stackIndex;

    QTableWidget *m_table;
};






#endif
