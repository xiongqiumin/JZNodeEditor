#ifndef JZNODE_SAMPLE_PROJECT_H_
#define JZNODE_SAMPLE_PROJECT_H_

#include "JZProject.h"

class SampleProject
{
public:
    SampleProject();
    ~SampleProject();

    void loadProject();
    void saveProject();
    bool run();

protected:    
    void newProject(QString project);
    void addClassFile(QString class_name,QString super, QString ui_file = QString());
    void addResources(QString path);
    bool copyDir(QString fromDir, QString toDir);

    QString loadUi(QString file);

    QString m_root;
    QString m_name;
    QString m_resources;
    JZProject m_project;    
};


#endif