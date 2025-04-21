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
    enum RegSetType {
        SrcRegIn,
        SrcRegOut,
        DstRegIn,
        DstRegOut,
    };

    void dumpFile(JZScriptFile *file);

    QString irToString(JZNodeIR *ir);
    QString toString(JZNodeIRParam param);
    QString dealCall(QString function);
    bool isFunctionInParam(JZNodeIRParam param);
    bool isIrSetReg(int pc, RegSetType regType);
    bool isRegInParam(JZNodeIRParam param);
    bool isRegOutParam(JZNodeIRParam param);

    QString functionDeclare(const JZFunction* func);
    QString paramDefine(const JZParamDefine* define);
    void dumpClass(JZScriptClassItem* class_item, QString& def, QString& impl);
    void dumpFunction(JZScriptItem* func_item,QString &def,QString &impl);
    QString tab(int count);

    QString m_dirPath;
    
    JZScriptItem* m_script;
    const JZNodeScript* m_scriptImpl;

    JZProject* m_project;
    JZNodeProgram *m_program;
    JZScriptEnvironment m_env;

    QVector<int> m_jumpList;
};

#endif
