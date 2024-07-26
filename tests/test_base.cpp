#include <QFile>
#include <QTest>
#include <QTextStream>
#include <QApplication>
#include <QDir>
#include <QTest>
#include "test_base.h"
#include "JZNodeBuilder.h"
#include "JZNodeUtils.h"

BaseTest::BaseTest()
{
    makeDump();
    m_file = nullptr;
    m_builder.setProject(&m_project);
    connect(&m_engine, &JZNodeEngine::sigRuntimeError, this, &BaseTest::onRuntimeError);
}

void BaseTest::makeDump()
{
    QString path = qApp->applicationDirPath() + "/dump";
    QDir dir;
    if(!dir.exists(path))
        dir.mkdir(path);
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
    m_project.clear();
    m_project.initProject("console");
    JZProject::setActive(&m_project);
    m_file = m_project.mainFile();
    m_engine.setDebug(false);
}

void BaseTest::cleanup()
{
    stop();
    JZProject::setActive(nullptr);
}

void BaseTest::onRuntimeError(JZNodeRuntimeError error)
{        
    qDebug().noquote() << m_program.dump();
    qDebug().noquote() << "Stack:\n" << error.errorReport();    
    Q_ASSERT(0);
}

void BaseTest::msleep(int ms)
{
    QThread::msleep(ms);
}

void BaseTest::dumpAsm(QString path)
{
    QString text = m_program.dump();
    QFile file(qApp->applicationDirPath() + "/dump/" + path);
    if(file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream s(&file);
        s << text;
        file.close();
    }
}

void BaseTest::dumpImage(JZScriptItem *func,QString file)
{   
    if(!file.endsWith(".png"))
        file += ".png";

    jzScriptItemUpdateLayout(func);
    jzScriptItemDump(func,qApp->applicationDirPath() + "/dump/" + file);
}

bool BaseTest::build()
{
    if(!m_builder.build(&m_program))
    {        
        QTest::qVerify(false, "build", m_builder.error().toLocal8Bit().data(), __FILE__, __LINE__);
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
        
    m_engine.deinit();
}

void BaseTest::asyncThread(QString name,QVariantList in)
{
    QVariantList out;
    m_callResult.ret = m_engine.call(name,in,out);
    if(m_callResult.ret)
        m_callResult.output = out;
}