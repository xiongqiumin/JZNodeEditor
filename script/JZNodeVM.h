#ifndef JZNODE_VM_H_
#define JZNODE_VM_H_

#include "JZNodeEngine.h"
#include "JZNodeDebugServer.h"

//JZNodeVM
class JZNodeVM : public QObject
{
    Q_OBJECT

public:
    JZNodeVM();
    ~JZNodeVM();
    
    bool init(QString path);    

    void addBreakPoint(int nodeId);
    void removeBreakPoint(int id);

    QVariant getVariable(QString name);
    void setVariable(QString name, const QVariant &value);        

protected slots:
    void onIntValueChanged(int value);
    void onStringValueChanged(const QString &value);
    void onDoubleValueChanged(double value);
    void onButtonClicked();    
    void onComboxSelectChanged(int index);
    void onValueNotify(int id,QVariant &value);

protected:
    virtual void customEvent(QEvent *event);        
    void createWindow();
    void dealEvent(JZEvent *event);

    QWidget *m_window;
    JZNodeEngine m_engine;
    JZNodeProgram m_program;
    JZNodeDebugServer m_debugServer;   
    bool m_debug;        
};








#endif
