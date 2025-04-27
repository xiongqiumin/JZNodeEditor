#ifndef JZ_COMM_MANAGER_H_
#define JZ_COMM_MANAGER_H_

#include "modbus/JZModuleModbus.h"
#include "3rd/JZCommon/jzModbus/JZModbusClient.h"
#include "3rd/JZCommon/jzModbus/JZModbusServer.h"

enum {
    Function_Bit,
    Function_InputBit,
    Function_InputRegister,
    Function_Register,
};

//JZCommModbusInfo
class JZCommModbusInfo
{
public:
    JZCommModbusInfo();

    QString name;
    JZModbusConnetInfo conn;
    QDataStream::ByteOrder bitOrder;
};
QDataStream& operator<<(QDataStream& s, const JZCommModbusInfo& param);
QDataStream& operator>>(QDataStream& s, JZCommModbusInfo& param);

//JZCommConfig
class JZCommConfig
{
public:
    QList<JZCommModbusInfo> modbusClient;
};
QDataStream &operator<<(QDataStream &s, const JZCommConfig &param);
QDataStream &operator>>(QDataStream &s, JZCommConfig &param);

//JZCommManager
class JZCommManager : public QObject
{
	Q_OBJECT

public:
    JZCommManager();
    ~JZCommManager();

	JZModbusClient* modbusClient(QString name);

    void init();

    void setConfig(const JZCommConfig &config);
    JZCommConfig config();

protected:
	QList<JZModbusClient*> m_modbusClient;
	QList<JZModbusServer*> m_modbusServer;
    JZCommConfig m_config;
};

void JZCommInit(JZCommManager* inst, const QByteArray& buffer);
JZVariantAny JZCommModbusRead(JZCommManager* mgr, const QString &name,const QJsonObject &param);
void JZCommModbusWrite(JZCommManager* mgr, const QString& name, const QJsonObject &param, JZVariantAny any);

QString JZCommTcpRead(JZCommManager* mgr, const QString& name, const QJsonObject& param);
void JZCommTcpWrite(JZCommManager* mgr, const QString& name, const QJsonObject& param, QString any);
QByteArray JZCommTcpReadBin(JZCommManager* mgr, const QString& name, const QJsonObject& param);
void JZCommTcpWriteBin(JZCommManager* mgr, const QString& name, const QJsonObject& param, const QByteArray &any);

QString JZCommUdpRead(JZCommManager* mgr, const QString& name, const QJsonObject& param);
void JZCommUdpWrite(JZCommManager* mgr, const QString& name, const QJsonObject& param, QString any);
QByteArray JZCommUdpReadBin(JZCommManager* mgr, const QString& name, const QJsonObject& param);
void JZCommUdpWriteBin(JZCommManager* mgr, const QString& name, const QJsonObject& param, const QByteArray& any);

QString JZCommSerialRead(JZCommManager* mgr, const QString& name, const QJsonObject& param);
void JZCommSerialWrite(JZCommManager* mgr, const QString& name, const QJsonObject& param, QString any);
QByteArray JZCommSerialReadBin(JZCommManager* mgr, const QString& name, const QJsonObject& param);
void JZCommSerialWriteBin(JZCommManager* mgr, const QString& name, const QJsonObject& param, const QByteArray& any);

#endif // !JZ_COMM_MANAGER_H_
