#ifndef JZ_VISION_APP_SAMPLE_H_
#define JZ_VISION_APP_SAMPLE_H_

#include "JZVisionAppNode.h"
#include "JZProject.h"

class JZVisionAppSample
{
public:
    JZVisionAppSample();
    
    void create(JZProject *project);
    void save(QString path);

protected:
    void updateInit();

    JZProject* m_project;
};

#endif