#include <QTest>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include "test_project.h"

ProjectTest::ProjectTest()
{
}

void ProjectTest::saveLoadNode()
{
    auto node_factory = m_project.environment()->nodeFactory();
    auto node_list = node_factory->nodeTypeList();
    for (int i = 0; i < node_list.size(); i++)
    {
        JZNode *node = node_factory->createNode(node_list[i]);
        QByteArray buffer = node_factory->saveNode(node);
        JZNode* node2 = node_factory->loadNode(buffer);
        QByteArray buffer2 = node_factory->saveNode(node2);
        delete node;
        delete node2;

        QVERIFY(buffer == buffer2);
    }
}

void ProjectTest::projectRename()
{
    QDir dir;
    dir.mkdir(qApp->applicationDirPath() + "/dump/test_saveProject");
    QString project_path = qApp->applicationDirPath() + "/dump/test_saveProject/pro.jsproj";

    JZFunctionDefine define;
    define.name = "hahahaha";
    JZScriptItem *script = m_file->addFunction(define);
    
    JZScriptItem *script2 = m_file->addFunction(define);
    QVERIFY(!script2);

    define.name = "hahahaha2";
    script2 = m_file->addFunction(define);
    QVERIFY(script2);
    for (int i = 0; i < 5; i++)
        script2->addNode(new JZNodeAdd());

    auto class_item = m_file->addClass("TetLu", "NXUX");

    JZScriptItem* flow = class_item->addFlow("flow");
    QVERIFY(flow);

    bool save_ret = m_project.saveAs(project_path);
    QVERIFY(save_ret);

    //save
    save_ret = m_project.saveAllItem();
    QVERIFY(save_ret);

    //rename
    m_project.renameItem(script, "nohahaha");
    QCOMPARE(script->name(), "nohahaha");
    script->addNode(new JZNodeAdd());
    QCOMPARE(script->nodeCount(), 2);

    m_project.renameItem(script2, "nohahaha");
    QCOMPARE(script2->name(), "hahahaha2");
    script2->addNode(new JZNodeAdd());
    QCOMPARE(script2->nodeCount(), 7);

    QString old_class_path = class_item->itemPath();
    class_item->setClass("NewClass", "NewSuper");
    m_project.saveItemMeta(old_class_path, class_item);

    m_project.renameItem(flow, "newFlow");
    
    //load and check
    bool open_ret = m_project.open(project_path);
    QVERIFY(open_ret);

    m_file = m_project.mainFile();
    script = (JZScriptItem*)m_file->getItem("nohahaha");
    QVERIFY(script);
    QCOMPARE(script->nodeCount(), 1);

    script2 = (JZScriptItem * )m_file->getItem("hahahaha2");
    QVERIFY(script2);
    QCOMPARE(script2->nodeCount(), 6);

    class_item = (JZScriptClassItem*)m_file->getItem("NewClass");
    QVERIFY(class_item);
    QCOMPARE(class_item->superClass(), "NewSuper");

    flow = (JZScriptItem*)class_item->flow("newFlow");
    QVERIFY(flow);
}

void test_project(int argc, char* argv[])
{
    ProjectTest test;
    QTest::qExec(&test, argc, argv);
}