#ifndef JZ_NODE_BUILDER_H_
#define JZ_NODE_BUILDER_H_

#include <QMutex>
#include <functional>
#include "JZNodeCompiler.h"

class JZNodeConstructBuild;

//JZNodeBuilder
class JZNodeBuilder
{
public:
    JZNodeBuilder();
    ~JZNodeBuilder();
    
    void setMute(bool mute);
    void log(int level, const QString& text);
    void logE(const QString& text);

    void setProject(JZProject *project);
    JZProject *project();

    void setScriptInclude(QList<JZScriptItem*> extList);
    void setScriptExclude(QList<JZScriptItem*> extList);

    bool build(JZNodeProgram *program);
    void stopBuild();
    bool isBuildInterrupt();

    bool isError() const;
    QString error() const;

    QMap<QString, CompilerResult> compilerResult();
    const CompilerResult *compilerInfo(JZScriptItem *file) const;

    void addClassInitFunction(QString class_name, ClassInitInfo function);

protected:    
    friend JZNodeConstructBuild;

    struct ClassConstructor 
    {
        QList<ClassInitInfo> infoList;        
    };

    struct ScriptInfo
    {
        JZNodeScriptPtr script;
        CompilerResult compilerInfo;
    };

    void clear();      
    bool buildScript(JZScriptItem *file, JZNodeScript* script);
    bool buildCustom(JZFunctionDefine define, JZNode *node, JZNodeScript* script_impl);
    bool link();        
    bool initGlobal();
    bool initConstructor();

    JZNodeProgram *m_program;    
    JZProject *m_project;    

    QMap<QString, ClassConstructor> m_classConstructor;
    QString m_checkError;
    QMap<QString, ScriptInfo> m_scripts;
    JZNodeCompiler m_compiler;
    
    QList<JZScriptItem*> m_scriptInclude;
    QList<JZScriptItem*> m_scriptExclude;

    QMutex m_mutex;
    bool m_logEnable;
    bool m_error;
    bool m_build;
    bool m_stopBuild;
};

#endif
