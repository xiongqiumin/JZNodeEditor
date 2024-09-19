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

    void init(JZProject *project,JZNodeProgram *program);
    JZNodeBuilder *builder();

    void startBuild();
    void stopBuild();

signals:
    void sigResult(bool flag);

protected:
    virtual void run() override;
    
    JZNodeProgram *m_program;
    JZNodeBuilder m_builder;
};

#endif
