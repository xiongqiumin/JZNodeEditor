#ifndef JZ_COMM_MANAGER_H_
#define JZ_COMM_MANAGER_H_

#include "modbus/JZModuleModbus.h"
#include "3rd/JZCommon/jzModbus/JZModbusClient.h"
#include "3rd/JZCommon/jzModbus/JZModbusServer.h"
#include "net/JZTcpClient.h"
#include "net/JZTcpServer.h"
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
    Comm_ModbusClient,
    Comm_ModbusServer,    
    Comm_TcpClient,
    Comm_TcpServer,
    Comm_Udp,
    Comm_SerialPort,
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

//JZCommConfig
class JZCommConfig
{
public:
    JZCommConfig();

    QString name;
    int commType;

    JZCommModbusInfo modbus;
    JZTcpClientInfo tcpClient;
    JZTcpServerInfo tcpServer;
    JZUdpInfo udp;
    JZSerialPortInfo serial;
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
    JZCommManager(QObject *parent = nullptr);
    ~JZCommManager();

    void init();
    void openAll();
    void closeAll();

    void setConfig(const JZCommManagerConfig& config);
    JZCommManagerConfig config();

	JZModbusClient* modbusClient(QString name);
    JZModbusServer* modbusServer(QString name);
    JZTcpClient* tcpClient(QString name);
    JZTcpServer* tcpServer(QString name);
    JZUdpSocket* udp(QString name);
    JZSerialPort* serial(QString name);

protected:
	QMap<QString, JZModbusClient*> m_modbusClient;
    QMap<QString, JZModbusServer*> m_modbusServer;
    QMap<QString, JZTcpClient*> m_tcpClient;
    QMap<QString, JZTcpServer*> m_tcpServer;
    QMap<QString, JZUdpSocket*> m_udp;
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
void JZCommUdpWrite(JZCommManager* mgr, const QString& name, const QByteArray& param, QString ip,int port);

QByteArray JZCommSerialRead(JZCommManager* mgr, const QString& name);
void JZCommSerialWrite(JZCommManager* mgr, const QString& name, const QByteArray& param);
QString JZCommSerialReadText(JZCommManager* mgr, const QString& name);
void JZCommSerialWriteText(JZCommManager* mgr, const QString& name, const QString& param);

#endif // !JZ_COMM_MANAGER_H_
