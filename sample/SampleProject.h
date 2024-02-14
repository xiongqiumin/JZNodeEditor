#ifndef JZNODE_SAMPLE_PROJECT_H_
#define JZNODE_SAMPLE_PROJECT_H_

#include "JZProject.h"

class SampleProject
{
public:
    SampleProject();
    ~SampleProject();

    void saveProject();
    bool run();

protected:    
    QString loadUi(QString file);

    QString m_name;
    JZProject m_project;    
};


#endif