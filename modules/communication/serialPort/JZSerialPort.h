#ifndef JZ_SERIAL_PORT_H_
#define JZ_SERIAL_PORT_H_

#include <QSerialPort>
#include "../JZCommPack.h"

//JZSerialPortInfo
class JZSerialPortInfo
{
public:
    JZSerialPortInfo();

    QString portName;
    int baud;
    QSerialPort::DataBits dataBit;
    QSerialPort::Parity parityBit;
    QSerialPort::StopBits stopBit;
};
QDataStream& operator<<(QDataStream& s, const JZSerialPortInfo& param);
QDataStream& operator>>(QDataStream& s, JZSerialPortInfo& param);

//JZSerialPort
class JZSerialPort : public QObject
{
    Q_OBJECT

public:
    JZSerialPort(QObject* parent = nullptr);
    ~JZSerialPort();

    void init(const JZSerialPortInfo &info);

    bool isOpen();
    bool open();
    void close();

    void write(const QByteArray &buffer);
    QByteArray read();

    void writeText(const QString &buffer);
    QString readText();
    
protected:
    JZSerialPortInfo m_info;

    JZCommPack m_pack;
    QSerialPort *m_com;
};



#endif