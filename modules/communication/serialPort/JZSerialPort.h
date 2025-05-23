#ifndef JZ_SERIAL_PORT_H_
#define JZ_SERIAL_PORT_H_

#include <QSerialPort>
#include "../JZCommPack.h"
#include "../JZComm.h"

//JZSerialPortConfig
class JZSerialPortConfig : public JZCommConfig
{
public:
    JZSerialPortConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    QString portName;
    int baud;
    QSerialPort::DataBits dataBit;
    QSerialPort::Parity parityBit;
    QSerialPort::StopBits stopBit;
};

//JZSerialPort
class JZSerialPort : public JZCommObject
{
    Q_OBJECT

public:
    JZSerialPort(QObject* parent = nullptr);
    ~JZSerialPort();

    void init(const JZSerialPortConfig&info);

    bool isOpen();
    bool open();
    void close();

    void write(const QByteArray &buffer);
    QByteArray read();

    void writeText(const QString &buffer);
    QString readText();
    
protected:
    JZSerialPortConfig m_info;

    JZCommPack m_pack;
    QSerialPort *m_com;
};



#endif