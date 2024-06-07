#ifndef JZNODE_GENERATE_CPP_H_
#define JZNODE_GENERATE_CPP_H_

#include "JZNodeProgram.h"
#include "JZProject.h"

class CppParam
{
public:
    QString type;
    QString name;
    QString value;
};

class CppFunction
{
public:
    QString declare();
    QString impl();

    QString name;
    QString className;
    QString returnType;
    QList<CppParam> params;

    QString code;
};

class CppClass
{
public:
    CppClass();
    ~CppClass();

    CppFunction *getFunction(QString name);
    QString declare();
    QString impl();

    QString name;
    QString super;
    QString uiClass;
    QString uiFile;
    QList<CppFunction*> functions;
    QList<CppParam> params;
};

class CppFile
{
public:
    CppFile();
    ~CppFile();

    CppClass *getClass(QString name);
    CppFunction *getFunction(QString name);

    QString path;
    QList<CppFunction*> functionList;
    QList<CppClass*> classList;
};

class CppUiFile
{
public:    
    QString path;
    QString xml;
};

class CppModule
{
public:
};

class CppProgram
{
public:
    CppProgram();
    ~CppProgram();
    
    CppFile *getFile(QString name);

    QList<CppParam> globals;
    QList<CppModule> modules;
    QList<CppFile> files;
    QList<CppUiFile> uis;
};

class JZNodeGenerateCpp
{
public:
    JZNodeGenerateCpp();
    ~JZNodeGenerateCpp();
    
    void generate(JZProject *project,QString output);

protected:
    bool toCppFunction(JZFunction *func,CppFunction *cfunc);
    bool dumpProgram(QString output);
    void saveFile(const QString &content,QString path);

    QString idName(int id);
    QString irToCpp(JZNodeIR *ir);
    QString paramToString(JZNodeIRParam param);
    
    JZProject *m_project;
    JZNodeProgram *m_program;
    JZFunction *m_function;

    CppProgram m_cppProgram;
    QList<int> m_jumpAddr;
    QStringList m_callInReg;
    QStringList m_callOutReg;
    QString m_macro;
    int m_tab;
};


#endif