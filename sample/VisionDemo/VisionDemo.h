#ifndef JZNODE_RUSSIAN_H_
#define JZNODE_RUSSIAN_H_

#include "../SampleProject.h"

class SampleVisionDemo : public SampleProject
{
public:
    SampleVisionDemo();
    ~SampleVisionDemo();

    void initCameraFile();
    void initCameraHik();

protected:    
    void initProject(QString name);
    void addInit();
    void addOnFrameReady();
    void addBtnClicked();
};



#endif // !JZNODE_RUSSIAN_H_
