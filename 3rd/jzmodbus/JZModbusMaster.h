#ifndef JZ_MODBUS_MASTER_H_
#define JZ_MODBUS_MASTER_H_

#include <QObject>
#include "JZModbusParam.h"
#include "JZModbusClient.h"

class JZModbusMaster : public QObject
{
    Q_OBJECT

public:
    JZModbusMaster();
    ~JZModbusMaster();    
    
    void setSlave(int slave);

    bool isOpen();
    bool isBusy();

    bool openRtu(QString com, int baud, QSerialPort::DataBits, QSerialPort::StopBits, QSerialPort::Parity);
    bool openTcp(QString ip, int port);
    void close();    
    
    JZModbusParamMap *map();
    void setStrategy(int addr, JZModbusStrategy s);
    void removeStrategy(int addr);

    void writeParam(int addr, QVariant value);
    QVariant readParam(int addr);

    void writeRemoteParam(int addr, QVariant value);
    QVariant readRemoteParam(int addr);

    void writeRemoteParamAsync(int addr, QVariant value);
    void readRemoteParamAsync(int addr);

signals:
    void sigParamChanged(int addr);
    void sigParamRecv(int addr);

protected slots:
    void onModbusReply(const JZModebusReply &reply);
    
protected:
    struct Command
    {
        enum {
            Read,
            Write,
        };

        int type;
        int addr;
        QVariant value;
    };

    struct Strategy
    {
        JZModbusStrategy s;
        qint64 readTime;
    };

    virtual void timerEvent(QTimerEvent *e) override;
    void dealCommand();    

    JZModbusParamMap m_map;
    JZModbusClient *m_client;
    int m_timeId;
    QMap<int, Strategy> m_strategyMap;
    QList<Command> m_commandList;
};


#endif