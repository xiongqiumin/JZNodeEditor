#ifndef JZ_MODEL_MANAGER_H_
#define JZ_MODEL_MANAGER_H_

#include <QDataStream>
#include "JZYolo.h"

class JZModelManagerConfig
{
public:    
    QList<JZModelConfigPtr> modelList;
};
QDataStream& operator<<(QDataStream &s,const JZModelManagerConfig &config);
QDataStream& operator>>(QDataStream &s, JZModelManagerConfig &config);

class JZModelManager : public QObject
{
    Q_OBJECT

public:
    JZModelManager(QObject* parent = nullptr);
    ~JZModelManager();
    
    void setConfig(const JZModelManagerConfig &config);
    JZModelManagerConfig config();

    void init();
    JZModel* model(QString name);

protected:
    JZModel* createModel(JZModelConfigPtr path);

    JZModelManagerConfig m_config;
    QList<JZModel*> m_models;
};
void JZModelInit(JZModelManager *inst,const QByteArray &buffer);
JZModel *JZModelGet(JZModelManager *inst, QString name);

#endif