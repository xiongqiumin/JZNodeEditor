#ifndef JZNODE_VM_H_
#define JZNODE_VM_H_

#include "JZNodeEngine.h"
#include "JZNodeDebugServer.h"

//JZNodeVM
class JZCORE_EXPORT JZNodeVM : public QObject
{
    Q_OBJECT

public:
    JZNodeVM();
    ~JZNodeVM();
    
    bool init(QString path,bool debug,QString &error);
    void quit();

protected slots:    
    void onRuntimeError(JZNodeRuntimeError error);

protected:
    virtual void customEvent(QEvent *event);
    void quitLater();
    
    JZNodeEngine m_engine;
    JZNodeProgram m_program;
    JZNodeDebugServer m_debugServer;   
    bool m_debug;    
};








#endif
