#include <stdexcept>
#include "JZCameraManager.h"
#include "JZNodeUtils.h"
#include "JZNodeEngine.h"
#include "JZNodeBind.h"
#include "JZCameraUVC.h"
#include "../JZModuleConfigFactory.h"

//JZCameraManagerConfig
QDataStream& operator<<(QDataStream& s, const JZCameraConfigPtr& param)
{
    JZModuleConfigFactory<JZCameraConfig>::instance()->saveToStream(s, param);
    return s;
}

QDataStream& operator>>(QDataStream& s, JZCameraConfigPtr& param)
{
    JZModuleConfigFactory<JZCameraConfig>::instance()->loadFromStream(s, param);
    return s;
}

int JZCameraManagerConfig::indexOfCamera(QString name) const
{
    for (int i = 0; i < cameraList.size(); i++)
    {
        if (cameraList[i]->name == name)
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
    m_cameras.clear();
    for (int i = 0; i < m_config.cameraList.size(); i++)
    {
        JZCamera *camera = createCamera(m_config.cameraList[i]);
        m_cameras.push_back(camera);
    }
}

const JZCameraManagerConfig &JZCameraManager::config() const
{
    return m_config;
}

void JZCameraManager::init()
{    
    emit sigInitFinish();
}

void JZCameraManager::addCamera(const JZCameraConfigPtr &config)
{    
    m_config.cameraList.push_back(config);
    m_cameras.push_back(createCamera(config));
}

void JZCameraManager::removeCamera(QString name)
{
    int idx = m_config.indexOfCamera(name);
    m_config.cameraList.removeAt(idx);
    auto camera = m_cameras[idx];
    m_cameras.removeAt(idx);
    if (camera->type() != Camera_Rtsp)
        delete camera;
    else
    {
        JZCameraRtsp *camera_rtsp = dynamic_cast<JZCameraRtsp*>(camera);
        camera_rtsp->stopAndDelete();
    }
}

bool JZCameraManager::open(QString name)
{
    auto c = camera(name);
    if (!c)
        return false;

    return c->open();
}

bool JZCameraManager::close(QString name)
{
    auto c = camera(name);
    if (!c)
        return false;

    c->close();
    return true;
}

bool JZCameraManager::start(QString name)
{
    auto c = camera(name);
    if (!c)
        return false;

    c->start();
    return true;
}

bool JZCameraManager::startOnce(QString name)
{
    auto c = camera(name);
    if (!c)
        return false;

    c->startOnce();
    return true;
}

bool JZCameraManager::stop(QString name)
{
    auto c = camera(name);
    if (!c)
        return false;

    c->stop();
    return true;
}

bool JZCameraManager::setCamera(QString name, const JZCameraConfigPtr &config)
{
    auto c = camera(name);
    if (!c)
        return false;

    return c->setConfig(config);    
}

QList<JZCamera*> JZCameraManager::cameraList()
{    
    return m_cameras;
}

JZCamera* JZCameraManager::camera(QString name)
{
    int idx = m_config.indexOfCamera(name);
    if (idx == -1)
        return NULL;

    return m_cameras[idx];
}

int JZCameraManager::indexOfCamera(QString name)
{
    return m_config.indexOfCamera(name);
}

JZCamera* JZCameraManager::createCamera(const JZCameraConfigPtr &config)
{
    bool open_ret = false;

    JZCamera *camera = nullptr;
    if(config->type == Camera_File)
    {
        JZCamera *camera_file = new JZCameraFile();
        camera = camera_file;
    }
    else if(config->type == Camera_UVC)
    {
        JZCameraUVC* camera_uvc = new JZCameraUVC();
        camera = camera_uvc;
    }
    else if(config->type == Camera_Hik)
    {
        JZCameraHik *camera_hik = new JZCameraHik();
        camera = camera_hik;
    }
    else if (config->type == Camera_Rtsp)
    {
        JZCameraRtsp *camera_rtsp = new JZCameraRtsp();
        camera = camera_rtsp;
    }
    else
    {
        Q_ASSERT(0);
    }
    
    camera->setObjectName(config->name);
    camera->setConfig(config);
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
    if (!camera->isOpen())
    {
        if (!camera->open())
            throw std::runtime_error("open camera failed");
    }

    camera->start();
}

void JZCameraStartOnce(JZCameraManager* inst, QString name)
{    
    JZCamera* camera = JZCameraGet(inst, name);
    if (!camera->isOpen())
    {
        if(!camera->open())
            throw std::runtime_error("open camera failed");
    }

    camera->startOnce();
}

void JZCameraStop(JZCameraManager* inst, QString name)
{
    JZCamera* camera = JZCameraGet(inst, name);
    camera->stop();
}