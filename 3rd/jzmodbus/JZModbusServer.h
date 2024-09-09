#ifndef JZ_MODBUS_SERVER_H_
#define JZ_MODBUS_SERVER_H_

#include <QObject>
#include <QSerialPort>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMap>
#include "JZModbusDefine.h"

class JZModbusContext;
class JZModbusServer : public QObject
{
    Q_OBJECT

public:
    JZModbusServer(QObject *parent = nullptr);
    ~JZModbusServer();

    void initMapping(int nb_bits, int nb_input_bits,
        int nb_registers, int nb_input_registers);
    JZModbusMapping *mapping();

    void setSlave(int slave);
    void setPlcMode(bool slave);

    void initRtu(QString com, int baud, QSerialPort::DataBits, QSerialPort::StopBits, QSerialPort::Parity);
    void initTcp(int port);
    bool isStart() const;
    bool start();
    void stop();

protected slots:
    void onComRead();
    void onNewConnect();
    void onTcpRead();
    void onTcpDisconnected();

signals:
    void sigMappingChanged(int addr,int nb);

protected:    
    int dealBuffer(uint8_t *req,QByteArray &buffer);
    void clear();

    JZModbusContext *m_ctx;
    
    QString m_comName;
    int m_baud;
    QSerialPort::DataBits m_dataBit;
    QSerialPort::StopBits m_stopBit;
    QSerialPort::Parity m_parityBit;
    int m_port;

    QSerialPort *m_com;
    QByteArray m_comBuffer;
    
    QTcpServer *m_tcpServer;
    QMap<QTcpSocket*, QByteArray> m_tcpClients;
    JZModbusMapping *m_mapping;
};

#endif