#ifndef JZ_SERIAL_PORT_H_
#define JZ_SERIAL_PORT_H_

#include <QSerialPort>
#include "../JZCommPack.h"

class JZSerialPort : public QObject
{
    Q_OBJECT

public:
    JZSerialPort(QObject* parent = nullptr);
    ~JZSerialPort();

    void open(QString com, int baud, QSerialPort::DataBits data_bit, QSerialPort::StopBits stop_bit, QSerialPort::Parity parity_bit);
    void close();

    void write(const QByteArray &buffer);
    QByteArray read();

    void writeText(const QString &buffer);
    QString readText();
    
protected:
    JZCommPack m_pack;
    QSerialPort *m_com;
};



#endif