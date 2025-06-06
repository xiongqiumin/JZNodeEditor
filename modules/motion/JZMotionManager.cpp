#include "JZMotionManager.h"
#include "JZNodeUtils.h"
#include "../JZModuleConfigFactory.h"
#include "JZMotionMotean.h"

//JZMotionManagerConfig
int JZMotionManagerConfig::indexOfMotion(QString name)
{
    for (int i = 0; i < motionList.size(); i++)
    {
        if (motionList[i]->name == name)
            return i;
    }
    return -1;
}

QDataStream& operator<<(QDataStream& s, const JZMotionManagerConfig& config)
{
	s << config.motionList;
	return s;
}

QDataStream& operator>>(QDataStream& s, JZMotionManagerConfig& config)
{
    s >> config.motionList;
	return s;
}

//JZMotionManager
JZMotionManager::JZMotionManager(QObject* parent)
	:QObject(parent)
{
}

JZMotionManager::~JZMotionManager()
{
	qDeleteAll(m_motionList);
	m_motionList.clear();
}

void JZMotionManager::setConfig(const JZMotionManagerConfig& config)
{
	m_config = config;
    init();
}

JZMotionManagerConfig JZMotionManager::config()
{
	return m_config;
}

void JZMotionManager::init()
{
    //clear
    qDeleteAll(m_motionList);
    m_motionList.clear();

    //init
	for (int i = 0; i < m_config.motionList.size(); i++)
	{
		JZMotion* model = createMotion(m_config.motionList[i]);
		m_motionList.push_back(model);
	}
}

JZMotion* JZMotionManager::motion(QString name)
{
	for (int i = 0; i < m_config.motionList.size(); i++)
	{
		if(m_config.motionList[i]->name == name)
			return m_motionList[i];
	}
	return nullptr;
}

QList<JZMotion*> JZMotionManager::motionList()
{
    return m_motionList;
}
	
JZMotion* JZMotionManager::createMotion(JZMotionConfigEnum config)
{
	JZMotion* motion = nullptr;
	if (config->type == Motion_Motean)
	{
        JZMotionMotean* yolo = new JZMotionMotean();
        motion = yolo;
	}
	else
	{
		Q_ASSERT(0);
	}
    
    motion->setConfig(config);
    motion->setParent(this);
	return motion;
}

void JZMotionInit(JZMotionManager* inst, const QByteArray& buffer)
{
    JZMotionManagerConfig config = JZNodeUtils::fromBuffer<JZMotionManagerConfig>(buffer);
	inst->setConfig(config);
}

JZMotion *JZMotionGet(JZMotionManager *inst, QString name)
{
    JZMotion *model = inst->motion(name);
    if (!model)
        throw std::runtime_error(qUtf8Printable("no model " + name));

    if (!model->isInit())
    {
        if(!model->init())
            throw std::runtime_error(qUtf8Printable("init model " + name + " failed"));
    }

    return model;
}