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
    m_debugServer.setEngine(&m_engine);
    m_window = nullptr;       

    connect(&m_engine,&JZNodeEngine::sigRuntimeError,this,&JZNodeVM::onRuntimeError);
}

JZNodeVM::~JZNodeVM()
{
    m_debugServer.stopServer();
    if(m_window)
        delete m_window;
}

bool JZNodeVM::init(QString path,bool debug)
{
    m_debug = debug;
    if(m_debug)     
    {
        m_debugServer.startServer(19888);
        m_debugServer.waitForAttach();
    }

    m_engine.setProgram(&m_program);    
    if(!m_program.load(path))
        return false;

    m_engine.init();    

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
