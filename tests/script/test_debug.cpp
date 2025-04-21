#include <QEventLoop>
#include <QDebug>
#include <math.h>
#include <QApplication>
#include <QTest>
#include "JZNodeFactory.h"
#include "JZNodeFlow.h"
#include "test_debug.h"

DebugTest::DebugTest()
{
}

void DebugTest::startServer(QString func, QVariantList input)
{
    m_engine.setDebug(true);

    m_server.setEngine(&m_engine);
    if (!m_server.startServer(18888))
    {
        QVERIFY2(false, "start server failded");
    }

    if (!m_client.connectToServer("127.0.0.1", 18888))
    {
        QVERIFY2(false, "connect to server failded");
    }
    QThread::msleep(100);

    JZNodeDebugInfo init_info;
    JZNodeProgramInfo ret;
    bool cmd_ret = m_client.init(init_info, ret);
    QVERIFY(cmd_ret);

    cmd_ret = m_server.waitForAttach(500);
    QVERIFY(cmd_ret);

    callAsync(func, input);
}

void DebugTest::stopServer()
{
    m_client.stop();
    m_server.stopServer();
}

void DebugTest::clearTestCase()
{
    BaseTest::clearTestCase();
    stopServer();
}

bool DebugTest::initWhileCase(QList<int> &id_list, QList<int> &value_list)
{
    auto script = m_project.mainFunction();
    JZNode *node_start = script->getNode(0);
    JZNodeWhile *node_while = new JZNodeWhile();
    JZNodeLiteral *node_true = new JZNodeLiteral();

    script->addLocalVariable("i", "int");

    script->addNode(node_while);
    script->addNode(node_true);

    node_true->setDataType(Type_bool);
    node_true->setLiteral("true");

    script->addConnect(JZNodeGemo(node_start->id(), node_start->flowOut()), JZNodeGemo(node_while->id(), node_while->flowIn()));
    script->addConnect(JZNodeGemo(node_true->id(), node_true->paramOut(0)), JZNodeGemo(node_while->id(), node_while->paramIn(0)));

    QList<JZNodeSetParam*> node_list;
    for (int i = 0; i < 100; i++)
    {
        JZNodeSetParam *node_set = new JZNodeSetParam();
        node_set->setVariable("i");
        node_set->setPinValue(node_set->paramIn(1), QString::number(i));
        script->addNode(node_set);

        id_list.push_back(node_set->id());
        value_list.push_back(i);
        if (i == 0)
            script->addConnect(JZNodeGemo(node_while->id(), node_while->subFlowOut(0)), JZNodeGemo(node_set->id(), node_set->flowIn()));
        else
        {
            JZNodeSetParam *pre_node = node_list.back();
            script->addConnect(JZNodeGemo(pre_node->id(), pre_node->flowOut()), JZNodeGemo(node_set->id(), node_set->flowIn()));
        }
        node_list.push_back(node_set);
    }

    return build();
}

void DebugTest::testBreakPoint()
{
    return;
    QList<int> id_list, value_list;
    if (!initWhileCase(id_list, value_list))
        return;
    dump("testBreakPoint");

    m_engine.setDebug(true);
    QVariantList in;
    callAsync("main", in);
    msleep(200);

    QString main_function = m_project.mainFunctionPath();
    for (int i = 0; i < 5; i++)
    {
        int id_index = rand() % id_list.size();
        int cur_id = id_list[id_index];
        int cur_value = value_list[id_index];

        m_engine.addBreakPoint(main_function, cur_id);
        msleep(50);
        QVERIFY(m_engine.status() == Status_pause);

        m_engine.stepOver();
        msleep(20);
        QVERIFY(m_engine.status() == Status_pause);

        int value = m_engine.getVariable("i").toInt();
        QCOMPARE(value, cur_value);

        m_engine.removeBreakPoint(main_function, cur_id);
        m_engine.resume();
    }

    m_engine.stop();
}

void DebugTest::testDebugServer()
{
    return;
    QString code = R"(int testWhile(int n) {
        int result = 0;
        int i = 0;
        while(i < n) {
            result = i;
            i = i + 1;
        }
        return result;
    })";

    if(!buildAs(code))
        return;
    dump("debug_testWhile");

    QVariantList in;
    in << 100;
    startServer("testWhile", in);
   
    bool cmd_ret = false;
    for (int i = 0; i < 10; i++)
    {
        cmd_ret = m_client.pause();
        QVERIFY(cmd_ret);

        msleep(10);
        
        cmd_ret = m_client.resume();
        QVERIFY(cmd_ret);

        msleep(50);
    }

    stopServer();

    QVERIFY(m_thread.output.size() == 1);
    QCOMPARE(m_thread.output[0].toInt(), 99);
}


void DebugTest::testBreakPointServer()
{
    QList<int> id_list, value_list;
    if (!initWhileCase(id_list, value_list))
        return;
    dump("testBreakPoint");
    
    QVariantList in;    
    startServer("main", in);

    BreakPoint pt;
    pt.type = BreakPoint::nodeEnter;
    pt.scriptItemPath = m_project.mainFunctionPath();
    
    bool ret = false;
    QString main_function = m_project.mainFunctionPath();
    for (int i = 0; i < 10; i++)
    {
        int id_index = rand() % id_list.size();
        int cur_id = id_list[id_index];
        int cur_value = value_list[id_index];

        pt.nodeId = cur_id;
        m_client.addBreakPoint(pt);
        msleep(100);
        QVERIFY(m_engine.status() == Status_pause);

        m_client.stepOver();
        msleep(100);
        QVERIFY(m_engine.status() == Status_pause);

        JZNodeRuntimeInfo runtime_info;
        ret = m_client.runtimeInfo(runtime_info);
        QVERIFY(ret);
        
        JZNodeGetDebugParam get_req;
        get_req.functionPath = runtime_info.stacks.back().scriptItemPath;
        get_req.coors << irRef("i");

        JZNodeGetDebugParamResp get_resp;
        ret = m_client.getVariable(get_req, get_resp);
        QVERIFY(ret);
        QCOMPARE(get_resp.values[0].value.toInt(), cur_value);

        m_client.removeBreakPoint(main_function, cur_id);
        m_client.resume();
    }

    stopServer();
}

void test_debug(int argc, char *argv[])
{    
    DebugTest s; 
    QTest::qExec(&s,argc,argv);
}
