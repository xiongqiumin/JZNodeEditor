#ifndef TEST_SERVER_H_
#define TEST_SERVER_H_

#include "JZProject.h"
#include "JZNodeEngine.h"
#include "JZNodeDebugServer.h"

class TestServer : public QThread
{
public:
    TestServer();  
    void stop();

protected:
    virtual void run() override;
    void init();

    JZProject m_project;
    JZNodeProgram m_program;        
};




















#endif // ! TEST_SERVER_H_H
