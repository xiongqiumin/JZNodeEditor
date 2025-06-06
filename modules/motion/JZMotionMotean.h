#ifndef JZ_MOTION_MOTEAN_H_
#define JZ_MOTION_MOTEAN_H_

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QQueue>
#include <QMap>
#include <QMutex>
#include <QDataStream>
#include "JZMotion.h"


//JZMotionMoteanConfig
class JZMotionMoteanConfig : public JZMotionConfig
{
public:
    JZMotionMoteanConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    QString portName;
    int baud;
    QSerialPort::DataBits dataBit;
    QSerialPort::Parity parityBit;
    QSerialPort::StopBits stopBit;
};

//JZMotionMoteam
class JZMotionMotean : public JZMotion
{
    Q_OBJECT

public:
    JZMotionMotean(QObject *parent = nullptr);
    ~JZMotionMotean();

    bool isInit();
    bool init();
    void deinit();

    bool isMoving() const;
    void gotoZero();
    void moveTo(double x, double y, double z);
};

#endif // PROTOCOL_H    