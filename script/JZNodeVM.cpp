#include "JZNodeVM.h"

//JZNodeVM
JZNodeVM::JZNodeVM()
{    
    m_debug = false;
}

JZNodeVM::~JZNodeVM()
{
}

bool JZNodeVM::load(QString path)
{
    if(!m_program.load(path))
        return false;

    m_engine.setProgram(m_program);    
    m_engine.init();
    if(m_debug)
    {
        m_debugServer.setEngine(this);
        m_debugServer.waitAttach();
    }
    createWindow();
    return true;
}

void JZNodeVM::customEvent(QEvent *event)
{
    if(m_state != Runner_running)
        return;

    JZEvent *e = (JZEvent *)event;
    auto &list = m_program->eventHandleList();
    for(int i = 0; i < list.size(); i++)
    {
        const JZEventHandle &handle = list[i];
        if(handle.match(e))
        {
            QVariantList in,out;
            auto func = handle.function;
            auto inList = func.paramIn;
            for (int i = 0; i < inList.size(); i++) 
            {
                //int id = m_program->paramId(func->n)  
                //setVariable(Reg_User + 1,paramIn[i]);    
            }                             
            call(func.name,in,out);
        }            
    }
}

void JZNodeVM::createWindow()
{
    QList<QPushButton*> btn_list;
    for(int i = 0; i < btn_list.size(); i++)
    {
        connect(btn_list[i],&QPushButton::clicked,this,JZNodeVM::onButtonClicked);
    }
}

void JZNodeVM::onValueNotify(int id,QVariant &value)
{

}

void JZNodeVM::onIntValueChanged(int value)
{
    JZEvent *event = new JZEvent();
    qApp->postEvent(this,event);
}

void JZNodeVM::onStringValueChanged(const QString &value)
{
    JZEvent *event = new JZEvent();
    qApp->postEvent(this,event);
}

void JZNodeVM::onDoubleValueChanged(double value)
{ 
    JZEvent *event = new JZEvent();
    qApp->postEvent(this,event);
}

void JZNodeVM::onButtonClicked()
{    
    JZEvent *event = new JZEvent();
    qApp->postEvent(this,event);    
}