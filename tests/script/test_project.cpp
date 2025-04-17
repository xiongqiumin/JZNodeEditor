#include <QTest>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include "test_project.h"

ProjectTest::ProjectTest()
{
}

void ProjectTest::saveLoad()
{
    auto main = m_project.mainFunction();
    JZNode* pre = main->startNode();
    for (int i = 0; i < 100; i++)
    {
        JZNode* nop = new JZNodeSetParam();
        main->addNode(nop);
        main->addConnect(pre->flowOutGemo(), nop->flowInGemo());
        pre = nop;
    }

    QString context = "nodes{\n";
    auto list = main->nodeList();
    for (int i = 0; i < list.size(); i++)
    {
        auto node = main->getNode(list[i]);
        context += node->name() + "{\"id\":8,\"values\":[\"\",\"\"] }" + "\n";
    }
    context += "}\n";
    
    context += "connects{\n";
    auto conn_list = main->connectList();
    for (int i = 0; i < conn_list.size(); i++)
    {
        auto& line = conn_list[i];
        QString text = QString::asprintf("%d.%d->%d.%d", line.from.nodeId, line.from.pinId, line.to.nodeId, line.to.pinId);
        context += text + "\n";
    }
    context += "}\n";
    

    qDebug().noquote() << context.size();
}

void test_project(int argc, char* argv[])
{
    ProjectTest test;
    QTest::qExec(&test, argc, argv);
}