#ifndef JZ_MOTION_MANAGER_H_
#define JZ_MOTION_MANAGER_H_

#include <QDataStream>
#include "JZMotion.h"

class JZMotionManagerConfig
{
public:    
    int indexOfMotion(QString name);

    QList<JZMotionConfigEnum> motionList;
};
QDataStream& operator<<(QDataStream &s,const JZMotionManagerConfig &config);
QDataStream& operator>>(QDataStream &s, JZMotionManagerConfig &config);

class JZMotionManager : public QObject
{
    Q_OBJECT

public:
    JZMotionManager(QObject* parent = nullptr);
    ~JZMotionManager();
    
    void setConfig(const JZMotionManagerConfig &config);
    JZMotionManagerConfig config();
    
    JZMotion* motion(QString name);
    QList<JZMotion*> motionList();

protected:
    void init();
    JZMotion* createMotion(JZMotionConfigEnum path);

    JZMotionManagerConfig m_config;
    QList<JZMotion*> m_motionList;
};
void JZMotionInit(JZMotionManager *inst,const QByteArray &buffer);
JZMotion *JZMotionGet(JZMotionManager *inst, QString name);

#endif