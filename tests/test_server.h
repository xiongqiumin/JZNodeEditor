#ifndef TEST_SERVER_H_
#define TEST_SERVER_H_

#include "JZProject.h"
#include "JZNodeEngine.h"
#include "JZNodeDebugServer.h"

class TestServer : public QThread
{
public:
    TestServer();  

    void init(JZProject *project);
    void stop();

protected slots:
    void onRuntimeError();

protected:
    virtual void run() override;    

    void addInitFunction();
    void addTimeoutFunction();
    
    JZProject *m_project;
    JZNodeProgram m_program;     
    JZNodeEngine *m_engine;
};




















#endif // ! TEST_SERVER_H_H
