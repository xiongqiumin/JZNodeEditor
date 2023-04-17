#ifndef JZ_NODE_BUILDER_H_
#define JZ_NODE_BUILDER_H_

#include "JZNodeCompiler.h"

//JZNodeBuilder
class JZNodeBuilder
{
public:
    JZNodeBuilder();
    ~JZNodeBuilder();

    bool build(JZProject *project,JZNodeProgram &result);   
    const QList<Graph*> &graphs(QString filename) const;

protected:
    bool buildScriptFile(JZScriptFile *script);    
    bool link();

    JZNodeProgram m_program;    
    JZProject *m_project;

    QMap<QString,ScriptInfo> m_scripts;
};











#endif