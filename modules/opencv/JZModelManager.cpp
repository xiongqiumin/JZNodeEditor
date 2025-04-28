#include "JZModelManager.h"
#include "JZNodeUtils.h"

//JZModelConfig
JZModelConfig::JZModelConfig()
{
	type = Model_None;
}
QDataStream& operator<<(QDataStream& s, const JZModelConfig& config)
{
	s << config.type << config.name << config.modelPath;
	return s;
}

QDataStream& operator>>(QDataStream& s, JZModelConfig& config)
{
	s >> config.type >> config.name >> config.modelPath;
	return s;
}


//JZModelManagerConfig
QDataStream& operator<<(QDataStream& s, const JZModelManagerConfig& config)
{
	s << config.models;
	return s;
}

QDataStream& operator>>(QDataStream& s, JZModelManagerConfig& config)
{
	s >> config.models;
	return s;
}

//JZModelManager
JZModelManager::JZModelManager()
{
}

JZModelManager::~JZModelManager()
{
	qDeleteAll(m_models);
	m_models.clear();
}

void JZModelManager::setConfig(const JZModelManagerConfig& config)
{
	m_config = config;
}

JZModelManagerConfig JZModelManager::config()
{
	return m_config;
}

void JZModelManager::init()
{
	for (int i = 0; i < m_config.models.size(); i++)
	{
		JZModel* model = createModel(m_config.models[i]);
		m_models.push_back(model);
	}
}

JZModel* JZModelManager::model(QString name)
{
	for (int i = 0; i < m_config.models.size(); i++)
	{
		if(m_config.models[i].name == name)
			return m_models[i];
	}
	return nullptr;
}
	
JZModel* JZModelManager::createModel(JZModelConfig config)
{
	if (config.type == Model_Yolo)
	{
		JZYolo* yolo = new JZYolo();
		yolo->loadNet(config.modelPath);
		return yolo;
	}

	Q_ASSERT(0);
	return nullptr;
}

void JZModelInit(JZModelManager* inst, const QByteArray& buffer)
{
	JZModelManagerConfig config = JZNodeUtils::fromBuffer<JZModelManagerConfig>(buffer);
	inst->setConfig(config);
	inst->init();
}

void JZModelForward(JZModelManager* inst, QString name)
{

}