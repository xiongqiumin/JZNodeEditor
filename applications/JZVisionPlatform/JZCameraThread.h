#ifndef JZ_CAMERA_THREAD_H_
#define JZ_CAMERA_THREAD_H_

#include <QThread>

class JZCameraThread : public QThread
{
    Q_OBJECT

public:
    JZCameraThread();
    ~JZCameraThread();
};




#endif