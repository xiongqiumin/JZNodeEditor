#ifndef JZNODE_PROGRAM_DUMPER_H_
#define JZNODE_PROGRAM_DUMPER_H_

#include <QThread>
#include "JZNodeProgram.h"
#include "JZScriptFile.h"

class JZNodeProgramDumper
{
public:
    JZNodeProgramDumper();

    void init(JZProject *project,JZNodeProgram *program);
    void dump(QString dirPath);

protected:
    void dumpFile(JZScriptFile *file);

    QString irToString(JZNodeIR *ir);
    QString toString(JZNodeIRParam param);
    QString dealCall(QString function);

    QString functionDeclare(JZFunction* func);
    QString paramDefine(const JZParamDefine* define);
    void dumpClass(JZScriptClassItem* class_item, QString& def, QString& impl);
    void dumpFunction(JZScriptItem* func_item,QString &def,QString &impl);
    QString tab(int count);

    QString m_dirPath;
    JZProject* m_project;
    JZNodeProgram *m_program;
    JZScriptEnvironment m_env;

    QVector<int> m_jumpList;
};

#endif
