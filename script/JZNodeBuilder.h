#ifndef JZ_NODE_BUILDER_H_
#define JZ_NODE_BUILDER_H_

#include "JZNodeCompiler.h"

//JZNodeBuilder
class JZNodeBuilder
{
public:
    JZNodeBuilder();
    ~JZNodeBuilder();

    bool build(JZProject *project,JZNodeProgram *program);
    QString error() const;

protected:    
    struct ConnectInfo
    {
        QString sender;
        int event;
        QString recv;
        QString handle;
    };

    void clear();
    bool buildScriptFile(JZScriptFile *script);    
    bool link();        
    void initGlobal();

    JZNodeProgram *m_program;    
    JZProject *m_project;    

    QMap<QString,JZNodeScriptPtr> m_scripts;   
    QList<ConnectInfo> m_connects;
    QString m_error;
};











#endif
