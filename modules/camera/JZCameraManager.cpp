#include <stdexcept>
#include "JZCameraManager.h"
#include "JZCameraFile.h"
#include "JZCameraHik.h"
#include "JZNodeUtils.h"
#include "JZNodeEngine.h"

//JZCameraConfig
JZCameraConfig::JZCameraConfig()
{
    type = Camera_None;
}
QDataStream &operator<<(QDataStream &s, const JZCameraConfig &param)
{
    s << param.type << param.name << param.path;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZCameraConfig &param)
{
    s >> param.type >> param.name >> param.path;
    return s;
}


//JZCameraManagerConfig
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
JZCameraManager::JZCameraManager()
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
}

JZCamera* JZCameraManager::camera(QString name)
{
    for(int i = 0; i < m_config.cameraList.size(); i++)
    {
        if(m_config.cameraList[i].name == name)
            return m_cameras[i];
    }

    return nullptr;
}


JZCamera* JZCameraManager::createCamera(const JZCameraConfig &config)
{
    bool open_ret = false;

    JZCamera *camera = nullptr;
    if(config.type == Camera_File)
    {
        JZCamera *camera_file = new JZCameraFile();
        open_ret = camera_file->open(config.path);

        camera = camera_file;
    }
    else if(config.type == Camera_Hik)
    {
        JZCamera *camera_hik = new JZCameraHik();
        open_ret = camera_hik->open(config.path);

        camera = camera_hik;
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

//JZCameraInit
void JZCameraInit(JZCameraManager* inst, const QByteArray& buffer)
{
    JZCameraManagerConfig config = JZNodeUtils::fromBuffer<JZCameraManagerConfig>(buffer);
    inst->setConfig(config);
    inst->init();
}

void JZCameraConnect(QObject *qrecv, JZCameraManager *inst,QString name, JZFunctionPointer func)
{    
    JZCamera *camera = inst->camera(name);
    JZNodeObject *sender = qobjectToJZObject(camera);
    JZNodeObject *recv = qobjectToJZObject(qrecv);
    JZObjectConnect(sender, JZFunctionPointer("sigFrameReady"), recv, func);
}