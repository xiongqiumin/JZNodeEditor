#ifndef JZ_MODEL_MANAGER_H_
#define JZ_MODEL_MANAGER_H_

#include <QDataStream>
#include "JZYolo.h"

class JZModelManagerConfig
{
public:    
    int indexOfModel(QString name);

    QList<JZModelConfigEnum> modelList;
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
    
    JZModel* model(QString name);
    QList<JZModel*> modelList();

protected:
    void init();
    JZModel* createModel(JZModelConfigEnum path);

    JZModelManagerConfig m_config;
    QList<JZModel*> m_models;
};
void JZModelInit(JZModelManager *inst,const QByteArray &buffer);
JZModel *JZModelGet(JZModelManager *inst, QString name);
QList<JZYoloResult> JZYoloForward(JZModelManager *inst, QString name, cv::Mat mat);

#endif