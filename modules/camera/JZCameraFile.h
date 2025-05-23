#ifndef JZ_CAMERA_FILE_H_
#define JZ_CAMERA_FILE_H_

#include "JZCamera.h"

class JZCameraFileConfig : public JZCameraConfig
{
public:
    JZCameraFileConfig();
    QString path;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;
};


class JZCameraFile : public JZCamera
{
    Q_OBJECT

public:
    JZCameraFile(QObject *parent = nullptr);
    ~JZCameraFile();

    virtual JZCameraType type() override;

    virtual bool setConfig(JZCameraConfigPtr config) override;
    virtual bool isOpen() override;
    virtual bool open() override;
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