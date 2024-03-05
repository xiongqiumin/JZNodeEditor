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
    bool buildUnitTest(JZScriptItem *file, ScriptDepend *depend, JZNodeProgram *program);    

    QString error() const;

protected:    
    friend JZNodeCustomBuild;

    struct ConnectInfo
    {
        QString sender;
        int event;
        QString recv;
        QString handle;
    };

    void clear();      
    bool buildScriptFile(JZScriptItem *script);
    bool buildCustom(JZFunctionDefine define,std::function<bool(JZNodeCompiler*, QString&)> func,const QList<JZParamDefine> &local = QList<JZParamDefine>());
    bool link();        
    void initGlobal();

    JZNodeProgram *m_program;    
    JZProject *m_project;    

    QMap<QString,JZNodeScriptPtr> m_scripts;   
    QList<ConnectInfo> m_connects;
    QString m_error;
};

#endif
