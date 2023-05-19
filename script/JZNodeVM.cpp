#include "JZNodeVM.h"
#include <QPushButton>
#include <QApplication>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>

//JZNodeVM
JZNodeVM::JZNodeVM()
{    
    m_debug = false;
    m_debugServer.setEngine(&m_engine);
}

JZNodeVM::~JZNodeVM()
{
}

bool JZNodeVM::init(QString path)
{
    if(m_debug)     
        m_debugServer.waitAttach();
    
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
    QList<QComboBox*> combo_list;
    QList<QLineEdit*> line_edit;
    QList<QSpinBox*> spin_list;
    QList<QDoubleSpinBox*> double_spin_list;
    QList<QPushButton*> btn_list;

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

void JZNodeVM::addBreakPoint(int nodeId)
{
        
}

void JZNodeVM::removeBreakPoint(int id)
{

}
