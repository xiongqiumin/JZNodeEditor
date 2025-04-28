#ifndef JZ_CAMERA_MANAGER_H_
#define JZ_CAMERA_MANAGER_H_

#include <QObject>
#include <QList>
#include <QDataStream>
#include "JZCamera.h"

//JZCameraConfig
class JZCameraConfig
{
public:
    JZCameraConfig();

    JZCameraType type;
    QString name;
    QString path;
};
QDataStream &operator<<(QDataStream &s, const JZCameraConfig &param);
QDataStream &operator>>(QDataStream &s, JZCameraConfig &param);

//JZCameraManagerConfig
class JZCameraManagerConfig
{
public:
    QList<JZCameraConfig> cameraList;
};
QDataStream &operator<<(QDataStream &s, const JZCameraManagerConfig &param);
QDataStream &operator>>(QDataStream &s, JZCameraManagerConfig &param);

//JZCameraManager
class JZCameraManager : public QObject
{
	Q_OBJECT

public:
    JZCameraManager();
    ~JZCameraManager();

	JZCamera* camera(QString name);

    void init();

    void setConfig(const JZCameraManagerConfig &config);
    JZCameraManagerConfig config();

protected:
    JZCamera*createCamera(const JZCameraConfig &config);

    JZCameraManagerConfig m_config;
    QList<JZCamera*> m_cameras;
};

void JZCameraInit(JZCameraManager* inst, const QByteArray& buffer);
void JZCameraConnect(QObject *object, const QByteArray &buffer);















#endif