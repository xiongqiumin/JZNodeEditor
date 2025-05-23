#include "JZModelManager.h"
#include "JZNodeUtils.h"
#include "../JZModuleConfigFactory.h"

//JZModelConfigPtr
QDataStream& operator<<(QDataStream& s, const JZModelConfigPtr& config)
{
	JZModuleConfigFactory<JZModelConfig>::instance()->saveToStream(s,config);
	return s;
}

QDataStream& operator>>(QDataStream& s, JZModelConfigPtr& config)
{
	JZModuleConfigFactory<JZModelConfig>::instance()->loadFromStream(s, config);
	return s;
}

//JZModelManagerConfig
QDataStream& operator<<(QDataStream& s, const JZModelManagerConfig& config)
{
	s << config.modelList;
	return s;
}

QDataStream& operator>>(QDataStream& s, JZModelManagerConfig& config)
{
	s >> config.modelList;
	return s;
}

//JZModelManager
JZModelManager::JZModelManager(QObject* parent)
	:QObject(parent)
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
	for (int i = 0; i < m_config.modelList.size(); i++)
	{
		JZModel* model = createModel(m_config.modelList[i]);
		m_models.push_back(model);
	}
}

JZModel* JZModelManager::model(QString name)
{
	for (int i = 0; i < m_config.modelList.size(); i++)
	{
		if(m_config.modelList[i]->name == name)
			return m_models[i];
	}
	return nullptr;
}
	
JZModel* JZModelManager::createModel(JZModelConfigPtr config)
{
	JZModel* model = nullptr;
	if (config->type == Model_Yolo)
	{
		JZYolo* yolo = new JZYolo();
		model = yolo;
	}
	else
	{
		Q_ASSERT(0);
	}

	model->setConfig(config);
	return model;
}

void JZModelInit(JZModelManager* inst, const QByteArray& buffer)
{
	JZModelManagerConfig config = JZNodeUtils::fromBuffer<JZModelManagerConfig>(buffer);
	inst->setConfig(config);
	inst->init();
}

JZModel *JZModelGet(JZModelManager *inst, QString name)
{
    JZModel *model = inst->model(name);
    if (!model)
        throw std::runtime_error(qUtf8Printable("no model " + name));

    return model;
}