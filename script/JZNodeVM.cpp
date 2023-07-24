#include "JZNodeVM.h"
#include <QPushButton>
#include <QApplication>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>

//JZNodeVM
JZNodeVM::JZNodeVM()
{    
    m_debug = false;        
    connect(&m_engine,&JZNodeEngine::sigRuntimeError,this,&JZNodeVM::onRuntimeError);
}

JZNodeVM::~JZNodeVM()
{
    m_debugServer.stopServer();
}

bool JZNodeVM::init(QString path,bool debug, QString &error)
{
    m_debug = debug;
    m_engine.setProgram(&m_program);
    if (!m_program.load(path))
    {
        error = m_program.error();
        return false;
    }

    m_engine.init();
    if (m_debug)
    {
        m_debugServer.setEngine(&m_engine);
        m_engine.setDebugger(&m_debugServer);

        m_debugServer.startServer(19888);
        m_debugServer.waitForAttach();

        auto debug_info = m_debugServer.debugInfo();
        auto it = debug_info.breakPoints.begin();
        while (it != debug_info.breakPoints.end())
        {
            auto filepath = it.key();
            auto list = it.value();
            for (int i = 0; i < list.size(); i++)
                m_engine.addBreakPoint(filepath, list[i]);
            it++;
        }
    }

    JZEvent event;
    event.eventType = Event_programStart;
    m_engine.dealEvent(&event);
    return true;
}

void JZNodeVM::customEvent(QEvent *event)
{    
    JZEvent *e = (JZEvent *)event;
}

void JZNodeVM::quit()
{
    m_engine.stop();
    qApp->exit();
}

void JZNodeVM::onRuntimeError(JZNodeRuntimeError error)
{
    QMessageBox::information(nullptr,"",error.info);
    QTimer::singleShot(0,[]{
       qApp->quit();
    });
}
