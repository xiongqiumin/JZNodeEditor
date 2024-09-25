#ifndef JZNODE_BUILD_THREAD_H_
#define JZNODE_BUILD_THREAD_H_

#include <QThread>
#include "JZNodeBuilder.h"

//JZNodeAutoRunWidget
class JZNodeBuildThread : public QThread
{
    Q_OBJECT

public:
    JZNodeBuildThread();
    ~JZNodeBuildThread();

    void init(JZNodeProgram *program);
    JZNodeBuilder *builder();

    void startBuild(JZProject *project);
    bool isBuild();
    void stopBuild();

signals:
    void sigResult(bool flag);

protected:
    virtual void run() override;
    void sync(JZProject *project);
    
    JZNodeProgram *m_program;
    JZNodeBuilder m_builder;
    JZProject m_project;
};

#endif
