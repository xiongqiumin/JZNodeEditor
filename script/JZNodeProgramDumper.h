#ifndef JZNODE_PROGRAM_DUMPER_H_
#define JZNODE_PROGRAM_DUMPER_H_

#include <QThread>
#include "JZNodeProgram.h"

class JZNodeProgramDumper
{
public:
    JZNodeProgramDumper();

    QString dump(JZNodeProgram *program);

protected:
    QString irToString(JZNodeIR *ir);
    QString toString(JZNodeIRParam param);
    
    JZNodeProgram *m_program;
    JZScriptEnvironment m_env;
};

#endif
