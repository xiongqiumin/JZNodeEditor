#ifndef JZNODE_AUTO_RUN_THREAD_H_
#define JZNODE_AUTO_RUN_THREAD_H_

#include <QWidget>
#include "JZNodeEngine.h"

//JZNodeAutoRunWidget
class JZNodeAutoRunThread : public QThread
{
    Q_OBJECT

public:
    JZNodeAutoRunThread();
    ~JZNodeAutoRunThread();

    void startRun(JZNodeProgram *program,const ScriptDepend &dpend);
    void stopRun();

signals:
    void sigResult(QSharedPointer<UnitTestResult> result);

protected:
    virtual void run() override;
    
    JZNodeEngine m_engine;
    JZNodeProgram *m_program;
    ScriptDepend m_depend;
    UnitTestResult ret;
};

#endif
