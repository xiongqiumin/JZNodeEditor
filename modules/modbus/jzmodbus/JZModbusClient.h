#ifndef JZ_MODBUS_RTU_CLIENT_H_
#define JZ_MODBUS_RTU_CLIENT_H_

#include <QObject>
#include <QSerialPort>
#include <QTcpSocket>
#include <QVector>
#include "JZModbusDefine.h"

class JZModbusContext;
class JZModebusReply
{
public:
    int code;
    int function;
    int addr;        
    QVector<uint8_t> bit;
    QVector<uint16_t> reg;
};

class JZModbusClient : public QObject
{
    Q_OBJECT

public:
    JZModbusClient(QObject *parent = nullptr);
    ~JZModbusClient();
   
    void initRtu(QString com,int baud, QSerialPort::DataBits, QSerialPort::StopBits, QSerialPort::Parity);
    void initTcp(QString ip, int port);
    bool open();
    void close();
    bool isOpen();
    
    bool setSlave(int salve);
    void setPlcMode(bool flag);

    QString error();
    bool isBusy();
    bool waitFinish();

    bool readBits(int addr, int nb, QVector<uint8_t> &dest);
    bool readInputBits(int addr, int nb, QVector<uint8_t> &dest);
    bool readRegisters(int addr, int nb, QVector<uint16_t> &dest);
    bool readInputRegisters(int addr, int nb, QVector<uint16_t> &dest);

    bool readBitsAsync(int addr, int nb);
    bool readInputBitsAsync(int addr, int nb);
    bool readRegistersAsync(int addr, int nb);
    bool readInputRegistersAsync(int addr, int nb);

    bool writeBit(int coil_addr, int status);
    bool writeBits(int addr, const QVector<uint8_t> &dest);
    bool writeRegister(int reg_addr, int value);
    bool writeRegisters(int addr, const QVector<uint16_t> &dest);

    bool writeBitAsync(int coil_addr, int status);
    bool writeBitsAsync(int addr, const QVector<uint8_t> &dest);
    bool writeRegisterAsync(int reg_addr, int value);
    bool writeRegistersAsync(int addr, const QVector<uint16_t> &dest);

signals:
    void sigModbusReplay(const JZModebusReply &reply);

protected slots:
    void onReadyRead();

protected:
    struct ReqInfo
    {
        void clear();

        int function;
        int addr;
        int nb;        
        qint64 reqTime;
        QByteArray buffer;
    };

    virtual void timerEvent(QTimerEvent *e) override;
    void clear();
    bool checkOpenBusy();
    void sendPacket(uint8_t *req, int req_length);
    JZModebusReply waitPacket();        
    int dealBuffer(JZModebusReply &info);    
    
    JZModbusContext *m_ctx;    
    ReqInfo m_req;
    QByteArray m_buffer;
    QString m_error;
    bool m_waitRecv;
    int m_timeId;

    QString m_comName;
    int m_baud; //波特率
    QSerialPort::DataBits m_dataBit;  //数据位
    QSerialPort::StopBits m_stopBit;  //停止位      
    QSerialPort::Parity m_parityBit;  //校验位

    QString m_ip;
    int m_port;

    QSerialPort *m_com;
    QTcpSocket *m_socket;
    QIODevice *m_io;
};



#endif // !JZ_MODBUS_CLIENT_H_
