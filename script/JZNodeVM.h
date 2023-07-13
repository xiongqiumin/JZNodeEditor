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
    
    bool init(QString path,bool debug);
    void quit();

protected slots:    
    void onRuntimeError(JZNodeRuntimeError error);

protected:
    virtual void customEvent(QEvent *event);                
    
    JZNodeEngine m_engine;
    JZNodeProgram m_program;
    JZNodeDebugServer m_debugServer;   
    bool m_debug;
    QMap<JZNodeObject*,JZNodeScript*> m_objectScripts;
    QMap<QObject*,JZNodeObject*> m_objects;    
};








#endif
