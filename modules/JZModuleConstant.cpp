#include <QSerialPort>
#include <QDataStream>
#include "JZModuleConstant.h"

JZModuleConstant *JZModuleConstant::instance()
{
    static JZModuleConstant inst;
    return &inst;
}

JZModuleConstant::JZModuleConstant()
{
    BitOrderValue = QList<int>{ QDataStream::LittleEndian, QDataStream::BigEndian };
    BitOrderText = QStringList{ "LittleEndian", "BigEndian" };

    BaudValue = QList<int>{ QSerialPort::Baud9600, QSerialPort::Baud19200, QSerialPort::Baud38400,
        QSerialPort::Baud57600, QSerialPort::Baud115200, };
    BaudText = QStringList{ "9600", "19200", "38400", "57600", "115200" };

    DataBitValue = QList<int>{ 5,6,7,8 };
    DataBitText = QStringList{ "5", "6", "7", "8" };

    ParityBitValue = QList<int>{ QSerialPort::NoParity, QSerialPort::EvenParity, QSerialPort::OddParity };
    ParityBitText = QStringList{ "No", "Even", "Odd" };

    StopBitValue = QList<int>{ QSerialPort::OneStop, QSerialPort::TwoStop };
    StopBitText = QStringList{ "OneStop", "TwoStop" };
}

JZModuleConstant::~JZModuleConstant()
{
}