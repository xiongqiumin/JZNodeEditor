#ifndef JZ_CAMERA_MANAGER_H_
#define JZ_CAMERA_MANAGER_H_

#include <QObject>
#include <QList>
#include <QDataStream>
#include "JZCamera.h"
#include "JZNodeType.h"
#include "JZCameraFile.h"
#include "JZCameraHik.h"
#include "JZCameraUVC.h"

//JZCameraManagerConfig
class JZCameraManagerConfig
{
public:
    int indexOfCamera(QString name);

    QList<JZCameraConfigPtr> cameraList;
};
QDataStream &operator<<(QDataStream &s, const JZCameraManagerConfig &param);
QDataStream &operator>>(QDataStream &s, JZCameraManagerConfig &param);

//JZCameraManager
class JZCameraManager : public QObject
{
	Q_OBJECT

public:
    JZCameraManager(QObject* parent = nullptr);
    ~JZCameraManager();

    QStringList cameraList();
	JZCamera* camera(QString name);

    void init();
    bool open(QString name);
    bool close(QString name);
    bool start(QString name);
    bool startOnce(QString name);
    bool stop(QString name);
    bool setCamera(QString name, JZCameraConfigPtr config);

    void setConfig(const JZCameraManagerConfig &config);
    JZCameraManagerConfig config();

signals:
    void sigInitFinish();

protected:
    JZCamera*createCamera(const JZCameraConfigPtr &config);

    JZCameraManagerConfig m_config;
    QList<JZCamera*> m_cameras;
};

void JZCameraConnect(QObject* qrecv, JZCameraManager* inst, QString name, JZFunctionPointer func);
void JZCameraInit(JZCameraManager* inst, const QByteArray& buffer);
void JZCameraStart(JZCameraManager* inst, QString name);
void JZCameraStartOnce(JZCameraManager* inst, QString name);
void JZCameraStop(JZCameraManager* inst, QString name);

#endif