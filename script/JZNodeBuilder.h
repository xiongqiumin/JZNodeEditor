#ifndef JZ_NODE_BUILDER_H_
#define JZ_NODE_BUILDER_H_

#include "JZNodeCompiler.h"
#include <functional>

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
    JZNodeBuilder(JZProject *project);
    ~JZNodeBuilder();
    
    bool build(JZNodeProgram *program);
    bool buildScript(JZScriptItem *file);
    bool buildUnitTest(JZScriptItem *file, ScriptDepend *depend, JZNodeProgram *program);    

    QString error() const;
    CompilerInfo compilerInfo(JZScriptItem *file) const;

protected:    
    friend JZNodeCustomBuild;

    struct ScriptInfo
    {
        JZNodeScriptPtr script;
        CompilerInfo compilerInfo;
    };

    struct ConnectInfo
    {
        QString sender;
        int event;
        QString recv;
        QString handle;
    };

    void clear();      
    bool buildCustom(JZFunctionDefine define,std::function<bool(JZNodeCompiler*, QString&)> func,const QList<JZParamDefine> &local = QList<JZParamDefine>());
    bool link();        
    void initGlobal();

    void replaceNopStatment(JZNodeScript *script, int index);
    void replaceUnitTestParam(JZScriptItem *file, ScriptDepend *depend);
    void replaceUnitTestFunction(JZScriptItem *script, ScriptDepend *depend);

    JZNodeProgram *m_program;    
    JZProject *m_project;    

    QMap<QString, ScriptInfo> m_scripts;

    QList<ConnectInfo> m_connects;
    QString m_error;
};

#endif
