#ifndef JZ_CAMERA_FILE_H_
#define JZ_CAMERA_FILE_H_

#include "JZCamera.h"

class JZCameraFile : public JZCamera
{
    Q_OBJECT

public:
    JZCameraFile(QObject *parent = nullptr);
    ~JZCameraFile();

    virtual bool open(QString path) override;
    virtual void close() override;

    virtual void start() override;
    virtual void startOnce() override;
    virtual void stop() override;

protected slots:
    void onReadTimer();

protected:
    cv::Mat readFrame();

    QTimer *m_timer;
    QStringList m_fileList;
    int m_fileIndex;
};

#endif