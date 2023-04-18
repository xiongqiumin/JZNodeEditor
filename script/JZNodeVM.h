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

    QWidget *mainWindow();
    bool load(QString path);       
    int exec();

    void addBreakPoint(BreakPoint pt);
    void removeBreakPoint(BreakPoint pt);

    QVariant getVariable(int id);
    void setVariable(int id, const QVariant &value);    

    void onValueNotify(int id,QVariant &value);

signals:
    void sigRuntimeError(JZNodeRuntimeError error);

protected:    
    void onIntValueChanged(int value);
    void onStringValueChanged(const QString &value);
    void onDoubleValueChanged(double value);
    void onButtonClicked();    

protected slots:    
    virtual void customEvent(QEvent *event);        
    void createWindow();
    bool busy();    

    QWidget *m_window;
    JZNodeEngine m_engine;
    JZNodeProgram m_program;
    JZNodeDebugServer m_debugServer;   
    bool m_debug;        
};








#endif
