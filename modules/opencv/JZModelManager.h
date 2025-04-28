#ifndef JZ_MODEL_MANAGER_H_
#define JZ_MODEL_MANAGER_H_

#include <QDataStream>
#include "JZYolo.h"

enum JZModelType
{
    Model_None,
    Model_Yolo,
};

class JZModelConfig
{
public:
    JZModelConfig();

    JZModelType type;
    QString name;
    QString modelPath;
};
QDataStream& operator<<(QDataStream& s, const JZModelConfig& config);
QDataStream& operator>>(QDataStream& s, JZModelConfig& config);

class JZModelManagerConfig
{
public:    
    QList<JZModelConfig> models;
};
QDataStream& operator<<(QDataStream &s,const JZModelManagerConfig &config);
QDataStream& operator>>(QDataStream &s, JZModelManagerConfig &config);

class JZModelManager
{
public:
    JZModelManager();
    ~JZModelManager();
    
    void setConfig(const JZModelManagerConfig &config);
    JZModelManagerConfig config();

    void init();
    JZModel* model(QString name);

protected:
    JZModel* createModel(JZModelConfig path);

    JZModelManagerConfig m_config;
    QList<JZModel*> m_models;
};
void JZModelInit(JZModelManager *inst,const QByteArray &buffer);

#endif