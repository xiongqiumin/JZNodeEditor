#ifndef JZNODE_AUTO_RUN_THREAD_H_
#define JZNODE_AUTO_RUN_THREAD_H_

#include <QWidget>
#include "JZScriptUnitTest.h"
#include "JZNodeEngine.h"

//JZNodeAutoRunWidget
class JZNodeAutoRunThread : public QThread
{
    Q_OBJECT

public:
    JZNodeAutoRunThread();
    ~JZNodeAutoRunThread();

    JZNodeEngine *engine();    
    JZScriptItemDepend *genDepend(JZScriptItem *script);

    void startRun();
    void stopRun();

signals:
    void sigResult(int result);

protected:
    virtual void run() override;
    
    JZScriptUnitTest m_test;
    bool m_cancel;
};

#endif
