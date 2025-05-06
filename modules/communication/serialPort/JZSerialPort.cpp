#include "JZSerialPort.h"

JZSerialPort::JZSerialPort(QObject* parent)
    :QObject(parent)
{
    m_com = new QSerialPort(this);
}

JZSerialPort::~JZSerialPort()
{
    close();
}

void JZSerialPort::open(QString com, int baud, QSerialPort::DataBits data_bit, QSerialPort::StopBits stop_bit, QSerialPort::Parity parity_bit)
{
    m_com->setPortName(com);
    if (!m_com->open(QIODevice::ReadWrite))
        return;

    m_com->setBaudRate(baud);
    m_com->setDataBits(data_bit);
    m_com->setStopBits(stop_bit);
    m_com->setParity(parity_bit);
}

void JZSerialPort::close()
{
    m_com->close();
}

void JZSerialPort::write(const QByteArray &buffer)
{
    QByteArray pack = m_pack.makePack(buffer);
    m_com->write(buffer);
}

QByteArray JZSerialPort::read()
{
    QByteArray buffer;
    while (true)
    {
        m_com->waitForReadyRead(20);
        if (m_com->bytesAvailable() > 0)
        {
            m_pack.appendBuffer(m_com->readAll());
            if (m_pack.takePack(buffer))
                break;
        }
    }
    return buffer;
}

void JZSerialPort::writeText(const QString &buffer)
{
    write(buffer.toUtf8());
}

QString JZSerialPort::readText()
{
    QByteArray buffer = read();
    return QString::fromUtf8(buffer);
}