#include <QFile>
#include <QTest>
#include <QTextStream>
#include <QApplication>
#include <QDir>
#include "test_base.h"
#include "JZNodeBuilder.h"
#include "JZNodeUtils.h"
#include "JZNodeProgramDumper.h"
#include "JZProjectTemplate.h"
#include "JZScriptConvert.h"
#include "JZNodeBind.h"

std::function<void()> g_testFunc;

//JZTestLambda
void JZTestLambda()
{
    g_testFunc();
}

void TestModule::regist(JZScriptEnvironment* env)
{
    env->functionManager()->registCFunction("JZTestLambda", true, jzbind::createFuncion(JZTestLambda));
}

void TestModule::unregist(JZScriptEnvironment* env)
{

}

//TestServer
TestServer::TestServer()
{
    m_project = nullptr;
    m_engine = nullptr;
}

void TestServer::init(JZProject* project)
{

}

void TestServer::addInitFunction()
{

}

void TestServer::addTimeoutFunction()
{

}

void TestServer::stop()
{
    if (m_engine)
        m_engine->stop();
    quit();
    wait();
}

void TestServer::onRuntimeError()
{
    quit();
}

void TestServer::run()
{

}

//EngineThread
EngineThread::EngineThread()
{
    engine = nullptr;
    mainThread = nullptr;
}

void EngineThread::start(QString func, QVariantList input_list)
{
    mainThread = QThread::currentThread();
    engine->moveToThread(this);
    function = func;
    input = input_list;
    QThread::start();
}

void EngineThread::run()
{
    engine->call(function, input, output);
    exec();
    engine->moveToThread(mainThread);
    mainThread = nullptr;
}

//BaseTest
BaseTest::BaseTest()
{
    makeDump();
    m_file = nullptr;
    m_builder.setProject(&m_project);
    connect(&m_engine, &JZNodeEngine::sigRuntimeError, this, &BaseTest::onRuntimeError);
    m_thread.engine = &m_engine;

    m_objInst = m_project.environment()->objectManager();
    m_funcInst = m_project.environment()->functionManager();
}

void BaseTest::makeDump()
{
    QString path = qApp->applicationDirPath() + "/dump";
    QDir dir;
    if (!dir.exists(path))
        dir.mkdir(path);

    m_dumpPath = path;
}

void BaseTest::callTest(QString function)
{
    int argc = 2;
    QVector<char*> argv;
    std::string std_func = function.toStdString();
    argv.push_back(nullptr);
    argv.push_back(&std_func[0]);

    QTest::qExec(this,argc,argv.data());
}


void BaseTest::initTestCase()
{

}

void BaseTest::init()
{
    resetTestCase();    
}

void BaseTest::cleanup()
{
    clearTestCase();
}

void BaseTest::resetTestCase()
{
    m_project.clear();
    JZProjectTemplate::instance()->initProject(&m_project, "console");
    m_file = m_project.mainFile();
    m_engine.setDebug(false);
}

void BaseTest::clearTestCase()
{
    stop();
}

void BaseTest::dump(QString name)
{
    JZNodeProgramDumper dumper;
    dumper.init(&m_project, &m_program);
    dumper.dump(m_dumpPath + "/" + name);
}

void BaseTest::onRuntimeError(JZNodeRuntimeError error)
{        
    dump("lastError");
    qDebug().noquote() << "Stack:\n" << error.errorReport();    
    Q_ASSERT(0);
}

void BaseTest::msleep(int ms)
{
    QThread::msleep(ms);
}

JZScriptClassItem *BaseTest::makeTestClass()
{
    auto class_item = m_file->addClass("TestClass", "QObject");
    m_project.addGlobalVariable("test_class", "TestClass");    

    return class_item;
}

QVariant BaseTest::testClassPointer()
{
    QVariant v = m_engine.getVariable("test_class");
    return JZNodeType::convertToPointer(v);
}

bool BaseTest::build()
{    
    m_project.registType();
    if(!m_builder.build(&m_program))
    {        
        QTest::qVerify(false, "build", qPrintable(m_builder.error()), __FILE__, __LINE__);
        return false;
    }        

    m_engine.setProgram(&m_program);
    m_engine.init();    
    return true;
}

bool BaseTest::buildAs(QString code)
{
    auto script_file = m_project.mainFile();

    JZScriptConvert convert;
    if (!convert.convertScript(code,m_file))
    {
        QTest::qVerify(false, "convert", convert.error().toLocal8Bit().data(), __FILE__, __LINE__);
        return false;
    }

    return build();
}

bool BaseTest::call(QString name,const QVariantList &in,QVariantList &out)
{
    bool ret = m_engine.call(name,in,out);
    return ret;
}

bool BaseTest::callMember(QString name, const QVariantList &in, QVariantList &out)
{
    QVariantList member_in = in;
    member_in.insert(0, testClassPointer());
    return call("TestClass::" + name, member_in, out);
}

void BaseTest::callAsync(QString name, const QVariantList& in)
{
    m_thread.start(name, in);
}

void BaseTest::stop()
{
    if(m_engine.isInit())
        m_engine.stop();
    if (m_thread.isRunning())
    {
        m_thread.exit();
        m_thread.wait();
    }
    if (m_engine.isInit())
        m_engine.deinit();
}

void BaseTest::asyncThread(QString name,QVariantList in)
{
    QVariantList out;
    m_callResult.ret = m_engine.call(name,in,out);
    if(m_callResult.ret)
        m_callResult.output = out;
}

//genCallUnitArg
CallArg genCallUnitArg(QStringList list)
{
    QSharedPointer<QByteArrayList> buffer = QSharedPointer<QByteArrayList>(new QByteArrayList());
    QByteArrayList &buffer_list = *buffer;
    buffer_list.push_back("app");
    for (int i = 0; i < list.size(); i++)
        buffer_list.push_back(list[i].toUtf8());

    int argc = buffer_list.size();
    QVector<char*> argv(argc);
    for (int i = 0; i < buffer_list.size(); i++)
        argv[i] = buffer_list[i].data();

    CallArg arg;
    arg.argc = argc;
    arg.argv = argv;
    arg.buffers = buffer;
    return arg;
}