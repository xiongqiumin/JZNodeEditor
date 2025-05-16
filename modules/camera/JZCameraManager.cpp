#include <stdexcept>
#include "JZCameraManager.h"
#include "JZNodeUtils.h"
#include "JZNodeEngine.h"
#include "JZNodeBind.h"
#include "JZCameraUVC.h"

//JZCamerHikConfig
QDataStream &operator<<(QDataStream &s, const JZCameraHikConfig &param)
{
    s << param.path;
    s << param.gain;
    s << param.exposureTime;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZCameraHikConfig &param)
{
    s >> param.path;
    s >> param.gain;
    s >> param.exposureTime;
    return s;
}

//JZCameraConfig
JZCameraConfig::JZCameraConfig()
{
    type = Camera_File;
}
QDataStream &operator<<(QDataStream &s, const JZCameraConfig &param)
{
    s << param.type << param.name;
    s << param.fileConfig;
    s << param.hikConfig;

    return s;
}

QDataStream &operator>>(QDataStream &s, JZCameraConfig &param)
{
    s >> param.type >> param.name;
    
    s >> param.fileConfig;
    s >> param.hikConfig;

    return s;
}


//JZCameraManagerConfig
int JZCameraManagerConfig::indexOfCamera(QString name)
{
    for (int i = 0; i < cameraList.size(); i++)
    {
        if (cameraList[i].name == name)
            return i;
    }
    return -1;
}

QDataStream &operator<<(QDataStream &s, const JZCameraManagerConfig &param)
{
    s << param.cameraList;
    return s;
}
QDataStream &operator>>(QDataStream &s, JZCameraManagerConfig &param)
{
    s >> param.cameraList;
    return s;
}

//JZCameraManager
JZCameraManager::JZCameraManager(QObject* parent)
    :QObject(parent)
{
}

JZCameraManager::~JZCameraManager()
{    
    for(int i = 0; i < m_cameras.size(); i++)
    {
        m_cameras[i]->close();
        delete m_cameras[i];
    }
    m_cameras.clear();
}

void JZCameraManager::setConfig(const JZCameraManagerConfig &config)
{    
    m_config = config;
}

JZCameraManagerConfig JZCameraManager::config()
{
    return m_config;
}

void JZCameraManager::init()
{
    for(int i = 0; i < m_config.cameraList.size(); i++)
    {
        JZCamera *camera = createCamera(m_config.cameraList[i]);
        m_cameras.push_back(camera);
    }
    emit sigInitFinish();
}

QStringList JZCameraManager::cameraList()
{
    QStringList cameras;
    for (int i = 0; i < m_cameras.size(); i++)
        cameras << m_cameras[i]->objectName();

    return cameras;
}

JZCamera* JZCameraManager::camera(QString name)
{
    int idx = m_config.indexOfCamera(name);
    if (idx == -1)
        return NULL;

    return m_cameras[idx];
}


JZCamera* JZCameraManager::createCamera(const JZCameraConfig &config)
{
    bool open_ret = false;

    JZCamera *camera = nullptr;
    if(config.type == Camera_File)
    {
        JZCamera *camera_file = new JZCameraFile();
        open_ret = camera_file->open(config.fileConfig.path);

        camera = camera_file;
    }
    else if(config.type == Camera_UVC)
    {
        JZCameraUVC* camera_file = new JZCameraUVC();
    }
    else if(config.type == Camera_Hik)
    {
        JZCameraHik *camera_hik = new JZCameraHik();
        camera = camera_hik;

        auto cfg = config.hikConfig;
        open_ret = camera_hik->open(cfg.path);
        if (open_ret)
        {
            camera_hik->setConfig(cfg);
        }
    }
    else
    {
        Q_ASSERT(0);
    }

    if(!open_ret)
        throw std::runtime_error("open camera failed");
    
    camera->setParent(this);
    return camera;
}

JZCamera* JZCameraGet(JZCameraManager* inst, QString name)
{
    JZCamera* camera = inst->camera(name);
    if (!camera)
        throw std::runtime_error("no camera");

    return camera;
}

//JZCameraInit
void JZCameraInit(JZCameraManager* inst, const QByteArray& buffer)
{
    JZCameraManagerConfig config = JZNodeUtils::fromBuffer<JZCameraManagerConfig>(buffer);
    inst->setConfig(config);
    inst->init();
}

void JZCameraConnect(QObject * object, JZCameraManager *inst,QString name, JZFunctionPointer func)
{    
    inst->connect(inst, &JZCameraManager::sigInitFinish, object, [=]{
        JZCamera* camera = JZCameraGet(inst,name);

        camera->connect(camera, &JZCamera::sigFrameReady, object, [object, func](cv::Mat mat)
        {
            JZNodeObject* jzobj = qobjectToJZObject(object);
            QVariantList in;
            jzbind::createSlotParams<int>(in, mat);
            jzobj->onSigTrigger(func.functionName(),in);
        });
    });
}

void JZCameraStart(JZCameraManager* inst, QString name)
{
    JZCamera* camera = JZCameraGet(inst, name);
    camera->start();
}

void JZCameraStartOnce(JZCameraManager* inst, QString name)
{
    JZCamera* camera = JZCameraGet(inst, name);
    camera->startOnce();
}

void JZCameraStop(JZCameraManager* inst, QString name)
{
    JZCamera* camera = JZCameraGet(inst, name);
    camera->stop();
}

void JZCameraSetting(JZCameraManager* inst, QString name)
{
    JZCamera* camera = JZCameraGet(inst, name);
}