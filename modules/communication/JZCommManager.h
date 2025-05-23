#ifndef JZ_COMM_MANAGER_H__
#define JZ_COMM_MANAGER_H__

#include "JZComm.h"
#include "modbus/JZCommModbusClient.h"
#include "modbus/JZCommModbusServer.h"
#include "net/JZTcpClient.h"
#include "net/JZTcpServer.h"
#include "net/JZUdpSocket.h"
#include "serialPort/JZSerialPort.h"
#include "JZNodeType.h"

enum {
    Function_Bit,
    Function_InputBit,
    Function_InputRegister,
    Function_Register,
};

//JZCommConfig
class JZCommManagerConfig
{
public:
    QList<JZCommConfigPtr> commList;
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

	JZCommModbusClient* modbusClient(QString name);
    JZCommModbusServer* modbusServer(QString name);
    JZTcpClient* tcpClient(QString name);
    JZTcpServer* tcpServer(QString name);
    JZUdpSocket* udp(QString name);
    JZSerialPort* serial(QString name);

protected:
    QObject* comm(QString name);
	QList<JZCommObject*> m_commList;

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
