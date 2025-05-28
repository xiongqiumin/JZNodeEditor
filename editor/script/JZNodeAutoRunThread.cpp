#include <QEvent>
#include <QCoreApplication>
#include <QTimer>
#include "JZNodeAutoRunThread.h"
#include "LogManager.h"

//JZNodeAutoRunEvent
class JZNodeAutoRunEvent : public QEvent
{
public:
    static int Event;

    enum
    {
        None,
        StartRun,
        StartRunOnce,
        StopRun,
        StopThread,
    };

    JZNodeAutoRunEvent()
        :QEvent((QEvent::Type)Event)
    {
        cmd = None;
    }

    virtual ~JZNodeAutoRunEvent()
    {
    }

    int cmd;
};
int JZNodeAutoRunEvent::Event = QEvent::registerEventType();

//JZNodeAutoRunThread
JZNodeAutoRunThread::JZNodeAutoRunThread()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &JZNodeAutoRunThread::onTimer);    

    moveToThread(this);
    m_test.moveToThread(this);
}

JZNodeAutoRunThread::~JZNodeAutoRunThread()
{
}

void JZNodeAutoRunThread::onTimer()
{
    if (m_test.isFinish())
    {
        JZScriptItemDepend *d = m_test.depend();
        if(d->status == JZScriptItemDepend::Successed)
            LOGMOD_I(Log_Runtime, "测试结束");
        else if (d->status == JZScriptItemDepend::Failed)
            LOGMOD_I(Log_Runtime, "测试失败:" + d->error);
        else if(d->status == JZScriptItemDepend::Cancel)
            LOGMOD_I(Log_Runtime, "测试中断");
        else {
            Q_ASSERT(0);
        }

        m_timer->stop();
        emit sigResult(d->status);
    }
}

JZScriptUnitTest *JZNodeAutoRunThread::unitTest()
{
    return &m_test;
}

JZNodeEngine *JZNodeAutoRunThread::engine()
{
    return m_test.engine();
}

void JZNodeAutoRunThread::customEvent(QEvent *e)
{
    JZNodeAutoRunEvent *event = dynamic_cast<JZNodeAutoRunEvent*>(e);    
    if (event->cmd == JZNodeAutoRunEvent::StartRun || event->cmd == JZNodeAutoRunEvent::StartRunOnce)
    {
        if (!m_test.isFinish())
            return;

        LOGMOD_I(Log_Runtime, "开始测试");
        if (!m_test.isInit())
        {
            if (!m_test.init())
                return;

            m_test.engine()->startWatch();
            m_timer->start(50);
        }

        bool is_once = false;
        if (event->cmd == JZNodeAutoRunEvent::StartRunOnce)
            is_once = true;
        
        m_test.start(is_once);
    }
    else if (event->cmd == JZNodeAutoRunEvent::StopRun)
    {        
        m_test.engine()->stopWatch();
        m_timer->stop();
    }
    else if (event->cmd == JZNodeAutoRunEvent::StopThread)
    {        
        m_test.deinit();
    }
}

void JZNodeAutoRunThread::startRun()
{        
    JZNodeAutoRunEvent *event = new JZNodeAutoRunEvent();
    event->cmd = JZNodeAutoRunEvent::StartRun;
    qApp->postEvent(this, event);
}

void JZNodeAutoRunThread::startRunOnce()
{
    JZNodeAutoRunEvent* event = new JZNodeAutoRunEvent();
    event->cmd = JZNodeAutoRunEvent::StartRunOnce;
    qApp->postEvent(this, event);
}

void JZNodeAutoRunThread::stopRun()
{    
    m_test.stop();

    JZNodeAutoRunEvent *event = new JZNodeAutoRunEvent();
    event->cmd = JZNodeAutoRunEvent::StopRun;
    qApp->postEvent(this, event);    
}

void JZNodeAutoRunThread::stopThread()
{
    stopRun();

    JZNodeAutoRunEvent *event = new JZNodeAutoRunEvent();
    event->cmd = JZNodeAutoRunEvent::StopThread;
    qApp->postEvent(this, event);

    QThread::msleep(50); //post后马上quit 不进入消息循环
    quit();
    wait();
}