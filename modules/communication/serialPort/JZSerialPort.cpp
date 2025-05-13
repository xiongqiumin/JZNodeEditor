#include <QDataStream>
#include "JZSerialPort.h"

//JZSerialPortInfo
JZSerialPortInfo::JZSerialPortInfo()
{
    portName = "COM1";
    baud = 9600;
    dataBit = QSerialPort::Data8;
    parityBit = QSerialPort::NoParity;
    stopBit = QSerialPort::OneStop;
}

QDataStream& operator<<(QDataStream& s, const JZSerialPortInfo& param)
{
    s << param.portName << param.baud << param.dataBit << param.parityBit << param.stopBit;
    return s;
}
QDataStream& operator>>(QDataStream& s, JZSerialPortInfo& param)
{
    s >> param.portName >> param.baud >> param.dataBit >> param.parityBit >> param.stopBit;
    return s;
}

//JZSerialPort
JZSerialPort::JZSerialPort(QObject* parent)
    :QObject(parent)
{
    m_com = new QSerialPort(this);
}

JZSerialPort::~JZSerialPort()
{
    close();
}

void JZSerialPort::init(const JZSerialPortInfo& info)
{
    m_info = info;
}

bool JZSerialPort::isOpen()
{
    return m_com->isOpen();
}

bool JZSerialPort::open()
{
    m_com->setPortName(m_info.portName);
    if (!m_com->open(QIODevice::ReadWrite))
        return false;

    m_com->setBaudRate(m_info.baud);
    m_com->setDataBits(m_info.dataBit);
    m_com->setStopBits(m_info.stopBit);
    m_com->setParity(m_info.parityBit);
    return true;
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