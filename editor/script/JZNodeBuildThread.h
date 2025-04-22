#ifndef JZNODE_BUILD_THREAD_H_
#define JZNODE_BUILD_THREAD_H_

#include <QThread>
#include "JZNodeBuilder.h"

enum {
    Build_Successed,
    Build_Failed,        
};

class JZNodeBuildResult
{
public:
    int status;
    JZNodeProgram program;
    QMap<QString, CompilerResult>  compilerResult;
};
typedef QSharedPointer<JZNodeBuildResult> JZNodeBuildResultPtr;

//JZNodeAutoRunWidget
class JZNodeBuildThread : public QThread
{
    Q_OBJECT

public:
    JZNodeBuildThread();
    ~JZNodeBuildThread();
    
    void setMute(bool mute);
    void startBuild(JZProject *project);
    bool isBuild();
    void stopBuild();

signals:
    void sigResult(JZNodeBuildResultPtr ptr);

protected:
    virtual void run() override;
    void sync(JZProject *project);
    
    JZProject m_project;
    JZNodeProgram m_program;
    JZNodeBuilder m_builder;        
};

#endif
