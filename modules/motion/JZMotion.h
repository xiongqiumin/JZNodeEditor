#ifndef JZ_MOTION_H_
#define JZ_MOTION_H_

#include <opencv2/opencv.hpp>
#include <QObject>
#include "../JZModuleConfigFactory.h"
#include "JZGCodeProgram.h"

enum JZMotionType
{
    Motion_None,    
    Motion_Simulator,
    Motion_Motean,
};

class JZMotionConfig
{
public:
    JZMotionConfig();

    QString name;
    int type;

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);
};
typedef JZModuleConfigEnum<JZMotionConfig> JZMotionConfigEnum;
QDataStream& operator<<(QDataStream& s, const JZMotionConfigEnum& config);
QDataStream& operator>>(QDataStream& s, JZMotionConfigEnum& config);

class JZMotion : public QObject
{
    Q_OBJECT

public:
    JZMotion(QObject *parent = nullptr);
    virtual ~JZMotion();

    QString name() const;
    const JZMotionConfigEnum &config();
    void setConfig(JZMotionConfigEnum config);

    virtual bool isInit() = 0;
    virtual bool init() = 0;
    virtual void deinit() = 0;

signals:
    void sigPos(double x, double y, double z);
    void sigTimeUpdated(double currentTime, double totalTime); // 时间更新信号
    void sigSimulationFinished();

protected:
    JZMotionConfigEnum m_config;
};

#endif // ! JZ_MOTION_H_
