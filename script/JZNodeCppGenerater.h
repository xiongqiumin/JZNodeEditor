#ifndef JZNODE_CPP_GENERATER_H_
#define JZNODE_CPP_GENERATER_H_

#include "JZNodeProgram.h"
#include "JZProject.h"

class CppFunction
{
public:
    JZFunctionDefine define;
    QString code;
};

class CppClass
{
public:
    CppClass();
    ~CppClass();

    CppFunction *getFunction(QString name);

    QString name;
    QString super;
    QString uiClass;
    QString uiFile;
    QList<CppFunction*> functions;
    QList<JZParamDefine> params;
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

    QList<JZParamDefine> globals;
    QList<CppModule> modules;
    QList<CppFile> files;
    QList<CppUiFile> uis;
};

class JZNodeCppGenerater
{
public:
    JZNodeCppGenerater();
    ~JZNodeCppGenerater();
    
    void generate(JZProject *project,QString output);

protected:
    struct NodeInfo
    {
        int dataType;
        bool isRef;
    };

    QString functionDeclare(CppFunction *func);
    QString functionImpl(CppFunction *func);
    QString classDeclare(CppClass *cls);
    QString classImpl(CppClass *cls);
    bool toCppFunction(JZFunction *func,CppFunction *cfunc);
    bool dumpProgram(QString output);
    void saveFile(const QString &content,QString path);

    QString tab();
    QString typeName(QString type);
    QString typeName(int type);
    QString idName(int id);
    
    QString irToCpp(JZNodeIR *ir);
    int paramType(const JZNodeIRParam &param);
    QString paramToString(const JZNodeIRParam &param);
    bool isRef(JZNodeIRParam param);

    JZFunctionDebugInfo *debugInfo();
    QString dealCall();
    
    JZProject *m_project;
    JZNodeProgram *m_program;
    JZFunction *m_function;
    JZNodeScript *m_script;
    JZScriptItem *m_file;
    QMap<int,NodeInfo> m_nodeType;

    CppProgram m_cppProgram;
    QList<int> m_jumpAddr;
    QString m_macro;
    
    int m_tab;

    JZNodeIRCall *m_irCall;
    QList<JZNodeIRParam> m_callInList;
    QList<JZNodeIRParam> m_callOutList;
    QList<JZNodeIRParam> m_returnList;
};


#endif