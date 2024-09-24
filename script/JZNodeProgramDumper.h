#ifndef JZNODE_PROGRAM_DUMPER_H_
#define JZNODE_PROGRAM_DUMPER_H_

#include <QThread>
#include "JZNodeProgram.h"

class JZCORE_EXPORT JZNodeProgramDumper
{
public:
    JZNodeProgramDumper();

    QString dump(JZNodeProgram *program);

protected:
    QString irToString(JZNodeIR *ir);
    QString toString(JZNodeIRParam param);

    JZScriptEnvironment m_env;    
    JZNodeProgram *m_program;
};

#endif
