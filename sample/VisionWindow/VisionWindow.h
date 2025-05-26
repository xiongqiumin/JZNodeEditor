#ifndef JZ_SAMPLE_VISION_WINDOW_H_
#define JZ_SAMPLE_VISION_WINDOW_H_

#include "../SampleProject.h"
#include "modules/vision/JZModuleVision.h"

class SampleVisionWindow : public SampleProject
{
public:
    SampleVisionWindow();
    ~SampleVisionWindow();

    void initCameraHik();
    void initCameraFile();
    void initCameraRtsp();

protected:    
    void initProject(QString name);
    void addInit();
    void addOnFrameReady();
};



#endif // !JZNODE_RUSSIAN_H_
