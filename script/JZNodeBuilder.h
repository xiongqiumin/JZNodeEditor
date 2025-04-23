#ifndef JZ_NODE_BUILDER_H_
#define JZ_NODE_BUILDER_H_

#include <QMutex>
#include <functional>
#include "JZNodeCompiler.h"

class JZNodeBuilder;
class JZNodeCustomBuild: public JZNode
{
public:    
    JZNodeCustomBuild();

    bool compiler(JZNodeCompiler *compiler, QString &error);    
    std::function<bool(JZNodeCompiler*, QString&)> buildFunction;    
};

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

    bool build(JZNodeProgram *program);
    void stopBuild();
    bool isBuildInterrupt();

    bool isError() const;
    QString error() const;

    QMap<QString, CompilerResult> compilerResult();
    const CompilerResult *compilerInfo(JZScriptItem *file) const;

protected:    
    friend JZNodeCustomBuild;

    struct ScriptInfo
    {
        JZNodeScriptPtr script;
        CompilerResult compilerInfo;
    };

    void clear();      
    bool buildScript(JZScriptItem *file);
    bool buildCustom(JZFunctionDefine define,std::function<bool(JZNodeCompiler*, QString&)> func);
    bool link();        
    bool initGlobal();

    JZNodeProgram *m_program;    
    JZProject *m_project;    

    QMap<QString, ScriptInfo> m_scripts;
    JZNodeCompiler m_compiler;

    bool m_logEnable;
    bool m_error;
    QMutex m_mutex;
    bool m_build;
    bool m_stopBuild;
};

#endif
