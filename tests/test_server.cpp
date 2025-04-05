#include <QApplication>
#include <QDir>
#include "test_server.h"
#include "JZNodeBuilder.h"
#include "JZNodeValue.h"
#include "JZNodeFunction.h"
#include "JZNodeUtils.h"
#include "LogManager.h"

TestServer::TestServer()
{    
    m_project = nullptr;
    m_engine = nullptr;
}

void TestServer::init(JZProject *project)
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
    if(m_engine)
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