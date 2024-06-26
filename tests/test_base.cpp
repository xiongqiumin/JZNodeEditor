#include <QFile>
#include <QTest>
#include <QTextStream>
#include "test_base.h"
#include "JZNodeBuilder.h"
#include "JZNodeUtils.h"

BaseTest::BaseTest()
{
    m_file = nullptr;
    connect(&m_engine, &JZNodeEngine::sigRuntimeError, this, &BaseTest::onRuntimeError);
}

void BaseTest::callTest(QString function)
{
    initTestCase();
    init();
    metaObject()->invokeMethod(this,qPrintable(function));
    cleanup();
}


void BaseTest::initTestCase()
{

}

void BaseTest::init()
{
    m_project.clear();
    m_project.initProject("console");
    m_file = m_project.mainFile();
}

void BaseTest::cleanup()
{
    stop();
}

void BaseTest::onRuntimeError(JZNodeRuntimeError error)
{        
    qDebug().noquote() << m_program.dump();
    qDebug() << error.error;    
    Q_ASSERT(0);
}

void BaseTest::msleep(int ms)
{
    QThread::msleep(ms);
}

void BaseTest::dumpAsm(QString path)
{
    QString text = m_program.dump();
    QFile file(path);
    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream s(&file);
        s << text;
        file.close();
    }
}

void BaseTest::dumpImage(QString func_name,QString file)
{   
    auto script_file = m_project.getScriptFile(m_project.mainFunction());
    auto func = script_file->getFunction(func_name);
    Q_ASSERT(func);
    jzScriptItemUpdateLayout(func);
    jzScriptItemDump(func,file);
}

bool BaseTest::build()
{
    JZNodeBuilder builder(&m_project);
    if(!builder.build(&m_program))
    {        
        QTest::qVerify(false, "build", builder.error().toLocal8Bit().data(), __FILE__, __LINE__);
        return false;
    }        

    m_engine.setProgram(&m_program);
    m_engine.init();
    
    return true;
}

bool BaseTest::call(QString name,const QVariantList &in,QVariantList &out)
{
    bool ret = m_engine.call(name,in,out);
    return ret;
}

void BaseTest::callAsync(QString name,const QVariantList &in)
{
    m_thread = std::thread(&BaseTest::asyncThread,this,name,in);
}

void BaseTest::stop()
{
    m_engine.stop();
    if(m_thread.joinable())
        m_thread.join();
}

void BaseTest::asyncThread(QString name,QVariantList in)
{
    QVariantList out;
    m_callResult.ret = m_engine.call(name,in,out);
    if(m_callResult.ret)
        m_callResult.output = out;
}