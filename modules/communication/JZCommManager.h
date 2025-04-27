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

//JZModbusManagerConfig
class JZModbusManagerConfig
{
public:
    QList<JZModbusConnetInfo> m_clientConnet;
};
QDataStream &operator<<(QDataStream &s, const JZModbusManagerConfig &param);
QDataStream &operator>>(QDataStream &s, JZModbusManagerConfig &param);

//JZCommManager
class JZCommManager : public QObject
{
	Q_OBJECT

public:
    JZCommManager();
    ~JZCommManager();

	JZModbusClient* modbusClient(int idx);

    void init();

    void setConfig(const JZModbusManagerConfig &config);
    JZModbusManagerConfig config();

protected:
	QList<JZModbusClient*> m_modbusClient;
	QList<JZModbusServer*> m_modbusServer;
    JZModbusManagerConfig m_config;
};

void JZCommInit(JZCommManager* inst, const QByteArray& buffer);
JZVariantAny JZCommModbusRead(JZModbusClient *client, const QJsonObject &param);
void JZCommModbusWrite(JZModbusClient *client, const QJsonObject &param, JZVariantAny any);

#endif // !JZ_COMM_MANAGER_H_
