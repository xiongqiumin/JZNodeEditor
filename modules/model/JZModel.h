#ifndef JZ_MODEL_H_
#define JZ_MODEL_H_

#include <QString>
#include <QSharedPointer>
#include "../JZModuleConfigFactory.h"

enum JZModelType
{
	Model_None,
	Model_Yolo,
};

class JZModelConfig
{
public:
	JZModelConfig();

	QString name;
	int type;

	virtual void saveToStream(QDataStream& s) const;
	virtual void loadFromStream(QDataStream& s);
};
typedef JZModuleConfigEnum<JZModelConfig> JZModelConfigEnum;

class JZModel
{
public:
	JZModel();
	virtual ~JZModel();

    QString name() const;
    const JZModelConfigEnum &config();
	void setConfig(JZModelConfigEnum config);

    virtual bool isInit() = 0;
	virtual bool init() = 0;

protected:
	JZModelConfigEnum m_config;
};

#endif