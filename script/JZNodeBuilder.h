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
    bool buildScriptFile(JZScriptFile *script);    
    bool link();    

    JZNodeProgram *m_program;    
    JZProject *m_project;

    QMap<QString,JZNodeScriptPtr> m_scripts;    
    QString m_error;
};











#endif
