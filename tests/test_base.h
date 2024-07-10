#ifndef TEST_BASE_H_
#define TEST_BASE_H_

#include <QObject>
#include <thread>
#include "JZProject.h"
#include "JZNodeEngine.h"
#include "JZNodeBuilder.h"

class BaseTest : public QObject
{
    Q_OBJECT

public:
    BaseTest();

    void callTest(QString function);

protected slots:
    void onRuntimeError(JZNodeRuntimeError error);

private slots:
    void initTestCase();
    void init();
    void cleanup();
    
protected:
    struct Promise
    {
        bool ret;
        QVariantList output;
    };
    
    bool build();
    bool call(QString name,const QVariantList &in,QVariantList &out);
    void callAsync(QString name,const QVariantList &in);
    void stop();
    void asyncThread(QString name,QVariantList in);  
    void dumpImage(JZScriptItem *item,QString file);
    void dumpAsm(QString file);
    void msleep(int ms);
    void makeDump();
    
    JZProject m_project;
    JZNodeProgram m_program;
    JZNodeEngine m_engine;
    JZScriptFile *m_file;
    JZNodeBuilder m_builder;
    Promise m_callResult;
    std::thread m_thread;
};









#endif