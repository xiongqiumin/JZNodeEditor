#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include <QPointer>
#include "JZNodeFactory.h"
#include "test_event.h"

EventTest::EventTest()
{
}

void EventTest::testTimer()
{
    auto class_item = m_file->addClass("testClass", "QObject");
    auto flow = class_item->addFlow("single_timer");
   
    m_project.addGlobalVariable("test", "testClass");
    m_project.addGlobalVariable("i", Type_int, "0");

    JZNodeTimerEvent* event = new JZNodeTimerEvent();
    event->setTimeOut(100);
    flow->addNode(event);

    JZNodeParam* param = new JZNodeParam();
    JZNodeSetParam* set_param = new JZNodeSetParam();
    JZNodeAdd* add = new JZNodeAdd();
    flow->addNode(param);
    flow->addNode(set_param);
    flow->addNode(add);

    param->setVariable("i");
    set_param->setVariable("i");

    flow->addConnect(param->paramOutGemo(0), add->paramInGemo(0));
    add->setParamInValue(1, "1");
    flow->addConnect(add->paramOutGemo(0), set_param->paramInGemo(1));
    flow->addConnect(event->flowOutGemo(), set_param->flowInGemo());

    if (!build())
        return;
    dump("event_testTimer");
  
    bool result = QTest::qWaitFor([this]()->bool 
    { 
        int i = m_engine.getVariable("i").toInt();
        return i >= 5;
    }, 1000);
    QVERIFY(result);

    m_engine.deinit();
    QTest::qWait(1000);
}

void EventTest::testMultiTimer()
{
    auto class_item = m_file->addClass("testClass", "QObject");
    m_project.addGlobalVariable("test", "testClass");
    m_project.addGlobalVariable("i", Type_int, "0");

    for (int i = 0; i < 4; i++)
    {
        auto flow = class_item->addFlow("testFlow" + QString::number(i));

        JZNodeTimerEvent* event = new JZNodeTimerEvent();
        event->setTimeOut(100);
        flow->addNode(event);

        JZNodeParam* param = new JZNodeParam();
        JZNodeSetParam* set_param = new JZNodeSetParam();
        JZNodeAdd* add = new JZNodeAdd();
        flow->addNode(param);
        flow->addNode(set_param);
        flow->addNode(add);

        param->setVariable("i");
        set_param->setVariable("i");

        flow->addConnect(param->paramOutGemo(0), add->paramInGemo(0));
        add->setParamInValue(1, "1");
        flow->addConnect(add->paramOutGemo(0), set_param->paramInGemo(1));
        flow->addConnect(event->flowOutGemo(), set_param->flowInGemo());
    }

    if (!build())
        return;
    dump("event_testMultiTimer");

    bool result = QTest::qWaitFor([this]()->bool
        {
            int i = m_engine.getVariable("i").toInt();
            return i >= 20;
        }, 1000);
    QVERIFY(result);
}


void test_event(int argc, char *argv[])
{    
    EventTest s; 
    QTest::qExec(&s,argc,argv);
}
