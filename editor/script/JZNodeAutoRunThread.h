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
    JZScriptUnitTest *unitTest();

    void startRun();
    void startRunOnce();
    void stopRun();
    void stopThread();

signals:
    void sigResult(int result);

protected slots:
    void onTimer();

protected:
    virtual void customEvent(QEvent *event) override;
    void init();

    QTimer *m_timer;
    bool m_watchFinish;
    JZScriptUnitTest m_test;
};

#endif
