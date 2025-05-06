#ifndef JZ_COMM_MANAGER_H_
#define JZ_COMM_MANAGER_H_

#include "modbus/JZModuleModbus.h"
#include "3rd/JZCommon/jzModbus/JZModbusClient.h"
#include "3rd/JZCommon/jzModbus/JZModbusServer.h"
#include "net/JZTcpClient.h"
#include "net/JZUdpSocket.h"
#include "serialPort/JZSerialPort.h"


enum {
    Function_Bit,
    Function_InputBit,
    Function_InputRegister,
    Function_Register,
};

enum {
    Comm_None,
    Comm_ModbusRtuClient,
    Comm_ModbusRtuServer,
    Comm_ModbusTcpClient,
    Comm_ModbusTcpServer,
    Comm_ModbusUdp,
    Comm_ModbusSerialPort,
};

//JZCommModbusInfo
class JZCommModbusInfo
{
public:
    JZCommModbusInfo();
    
    JZModbusConnetInfo conn;
    QDataStream::ByteOrder bitOrder;
};
QDataStream& operator<<(QDataStream& s, const JZCommModbusInfo& param);
QDataStream& operator>>(QDataStream& s, JZCommModbusInfo& param);

//JZCommTcpClientInfo
class JZCommTcpClientInfo
{
public:
    JZCommTcpClientInfo();

    QString ip;
    int port;

};
QDataStream& operator<<(QDataStream& s, const JZCommTcpClientInfo& param);
QDataStream& operator>>(QDataStream& s, JZCommTcpClientInfo& param);

//JZCommTcpServerInfo
class JZCommTcpServerInfo
{
public:
    JZCommTcpServerInfo();

    QString ip;
    int port;
};
QDataStream& operator<<(QDataStream& s, const JZCommTcpServerInfo& param);
QDataStream& operator>>(QDataStream& s, JZCommTcpServerInfo& param);

//JZCommUdpInfo
class JZCommUdpInfo
{
public:
    JZCommUdpInfo();

    int port;
};
QDataStream& operator<<(QDataStream& s, const JZCommUdpInfo& param);
QDataStream& operator>>(QDataStream& s, JZCommUdpInfo& param);

//JZCommSerialPortInfo
class JZCommSerialPortInfo
{
public:
    JZCommSerialPortInfo();

    QString portName;
    int baud;
    QSerialPort::DataBits dataBit;
    QSerialPort::Parity parityBit;
    QSerialPort::StopBits stopBit;
};
QDataStream& operator<<(QDataStream& s, const JZCommSerialPortInfo& param);
QDataStream& operator>>(QDataStream& s, JZCommSerialPortInfo& param);

//JZCommConfig
class JZCommConfig
{
public:
    JZCommConfig();

    QString name;
    int commType;

    JZCommModbusInfo modbus;
    JZCommTcpClientInfo tcpClient;
    JZCommTcpServerInfo tcpServer;
    JZCommUdpInfo udp;
    JZCommSerialPortInfo serial;
};
QDataStream &operator<<(QDataStream &s, const JZCommConfig &param);
QDataStream &operator>>(QDataStream &s, JZCommConfig &param);

//JZCommConfig
class JZCommManagerConfig
{
public:
    QList<JZCommConfig> commList;
};
QDataStream &operator<<(QDataStream &s, const JZCommManagerConfig &param);
QDataStream &operator>>(QDataStream &s, JZCommManagerConfig &param);

//JZCommManager
class JZCommManager : public QObject
{
	Q_OBJECT

public:
    JZCommManager();
    ~JZCommManager();

	JZModbusClient* modbusClient(QString name);
    JZTcpClient* tcpClient(QString name);
    JZUdpSocket* udpClient(QString name);
    JZSerialPort* serial(QString name);

    void init();

    void setConfig(const JZCommManagerConfig &config);
    JZCommManagerConfig config();

protected:
	QMap<QString,JZModbusClient*> m_modbusClient;
    QMap<QString,JZModbusServer*> m_modbusServer;
    QMap<QString, JZTcpClient*> m_tcpClient;
    QMap<QString, JZUdpSocket*> m_udpClient;
    QMap<QString, JZSerialPort*> m_serialPort;

    JZCommManagerConfig m_config;
};

void JZCommInit(JZCommManager* inst, const QByteArray& buffer);
JZVariantAny JZCommModbusRead(JZCommManager* mgr, const QString &name,int funcType,const QString &data_type,int addr);
void JZCommModbusWrite(JZCommManager* mgr, const QString& name, int funcType, const QString& data_type, int addr, JZVariantAny any);

QByteArray JZCommTcpRead(JZCommManager* mgr, const QString& name);
void JZCommTcpWrite(JZCommManager* mgr, const QString& name, const QByteArray& param);
QString JZCommTcpReadText(JZCommManager* mgr, const QString& name);
void JZCommTcpWriteText(JZCommManager* mgr, const QString& name, const QString& param);

QByteArray JZCommUdpRead(JZCommManager* mgr, const QString& name);
void JZCommUdpWrite(JZCommManager* mgr, const QString& name, const QByteArray& param);

QByteArray JZCommSerialRead(JZCommManager* mgr, const QString& name);
void JZCommSerialWrite(JZCommManager* mgr, const QString& name, const QByteArray& param);
QString JZCommSerialReadText(JZCommManager* mgr, const QString& name);
void JZCommSerialWriteText(JZCommManager* mgr, const QString& name, const QString& param);

#endif // !JZ_COMM_MANAGER_H_
