#ifndef JZNODE_SAMPLE_PROJECT_H_
#define JZNODE_SAMPLE_PROJECT_H_

#include "JZProject.h"
#include "JZNodeFlow.h"
#include "JZNodeValue.h"
#include "JZNodeOperator.h"
#include "JZNodeFunction.h"
#include "JZNodeExpression.h"

class SampleProject
{
public:
    SampleProject();
    ~SampleProject();

    JZProject *project();
    void loadProject();
    void saveProject();
    int run();

protected:    
    void newProject(QString project);    
    void addResources(QString path);
    bool copyDir(QString fromDir, QString toDir);

    QString loadUi(QString file);

    QString m_root;
    QString m_name;
    QString m_resources;
    JZProject m_project;    
    JZNodeObjectManager *m_objInst;
    JZNodeFunctionManager *m_funcInst;
};


#endif