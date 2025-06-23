#ifndef JZ_VISION_APP_SAMPLE_H_
#define JZ_VISION_APP_SAMPLE_H_

#include "JZVisionAppNode.h"
#include "JZProject.h"

class MainWindow;
class JZVisionAppSample
{
public:
    JZVisionAppSample();

    void updateInit();
    void create(MainWindow *mainwindow, QString path);

protected:
    JZProject* m_project;
};

#endif