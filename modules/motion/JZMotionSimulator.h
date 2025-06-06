#ifndef JZ_MOTION_SIMULATOR_H_
#define JZ_MOTION_SIMULATOR_H_

#include <QObject>
#include <QTimer>
#include <QQueue>
#include <QMap>
#include <QMutex>
#include <QDataStream>
#include "JZMotion.h"


//JZMotionSimulatorConfig
class JZMotionSimulatorConfig : public JZMotionConfig
{
public:
    JZMotionSimulatorConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);
};

//JZMotionMoteam
class JZMotionSimulator : public JZMotion
{
    Q_OBJECT

public:
    JZMotionSimulator(QObject *parent = nullptr);
    ~JZMotionSimulator();

    bool isInit();
    bool init();
    void deinit();

    bool isMoving() const;
    void gotoZero();
    void moveTo(double x, double y, double z);
};

#endif // PROTOCOL_H    