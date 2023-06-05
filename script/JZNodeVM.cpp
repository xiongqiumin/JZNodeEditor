#include "JZNodeVM.h"
#include <QPushButton>
#include <QApplication>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QMainWindow>
#include <QMessageBox>

//JZNodeVM
JZNodeVM::JZNodeVM()
{    
    m_debug = false;
    m_debugServer.setEngine(&m_engine);
    m_window = nullptr;
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
    createWindow();

    JZEvent event;
    event.setEventType(Event_programStart);
    dealEvent(&event);
    return true;
}

void JZNodeVM::customEvent(QEvent *event)
{    
    JZEvent *e = (JZEvent *)event;
    dealEvent(e);    
}

void JZNodeVM::createWindow()
{
    m_window = new QWidget();
    m_window->show();

    QLineEdit *line = new QLineEdit();
    line->setGeometry(30,30,120,30);

    QPushButton *btn = new QPushButton(m_window);
    btn->setText("click me");
    btn->setGeometry(30,30,120,60);

    QList<QComboBox*> combo_list;
    QList<QLineEdit*> line_edit;
    QList<QSpinBox*> spin_list;
    QList<QDoubleSpinBox*> double_spin_list;
    QList<QPushButton*> btn_list;
    line_edit << line;
    btn_list << btn;

    for(int i = 0; i < combo_list.size(); i++)
    {        
        connect(double_spin_list[i],SIGNAL(currentIndexChanged(int)),this,SLOT(onComboxSelectChanged(int)));
    }
    for(int i = 0; i < line_edit.size(); i++)
    {
        connect(line_edit[i],&QLineEdit::textChanged,this,&JZNodeVM::onStringValueChanged);
    }
    for(int i = 0; i < spin_list.size(); i++)
    {
        connect(double_spin_list[i],SIGNAL(valueChanged(int)),this,SLOT(onIntValueChanged(int)));        
    }
    for(int i = 0; i < double_spin_list.size(); i++)
    {   
        connect(double_spin_list[i],SIGNAL(valueChanged(double)),this,SLOT(onDoubleValueChanged(double)));
    }
    for(int i = 0; i < btn_list.size(); i++)
    {
        connect(btn_list[i],&QPushButton::clicked,this,&JZNodeVM::onButtonClicked);
    }
}

void JZNodeVM::onValueNotify(int id,QVariant &value)
{        
}

void JZNodeVM::onIntValueChanged(int value)
{
    JZEvent *event = new JZEvent();
    event->setEventType(Event_valueChanged);
    qApp->postEvent(this,event);
}

void JZNodeVM::onStringValueChanged(const QString &value)
{    
    JZEvent *event = new JZEvent();
    event->setEventType(Event_valueChanged);
    qApp->postEvent(this,event);
}

void JZNodeVM::onDoubleValueChanged(double value)
{ 
    JZEvent *event = new JZEvent();
    event->setEventType(Event_valueChanged);
    qApp->postEvent(this,event);
}

void JZNodeVM::onButtonClicked()
{    
    JZEvent *event = new JZEvent();
    event->setEventType(Event_buttonClicked);
    qApp->postEvent(this,event);    
}

void JZNodeVM::onComboxSelectChanged(int index)
{
    JZEvent *event = new JZEvent();
    event->setEventType(Event_comboxSelectChanged);
    qApp->postEvent(this,event);  
}

void JZNodeVM::dealEvent(JZEvent *event)
{
    auto list = m_program.matchEvent(event);
    for(int i = 0; i < list.size(); i++)
    {
        const JZEventHandle *handle = list[i];              
        QVariantList in = event->params;
        QVariantList out;           
        m_engine.call(&handle->function,in,out);
    }
}

void JZNodeVM::quit()
{
    m_engine.stop();
    qApp->exit();
}
