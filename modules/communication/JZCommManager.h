#ifndef JZ_COMM_MANAGER_H_
#define JZ_COMM_MANAGER_H_

#include "modbus/JZModuleModbus.h"
#include "3rd/JZCommon/jzModbus/JZModbusClient.h"
#include "3rd/JZCommon/jzModbus/JZModbusServer.h"

class JZCommManager : public QObject
{
	Q_OBJECT

public:
	int indexOfModbusClient(JZModbusConnetInfo config);
	JZModbusClient* newModbusClient(JZModbusConnetInfo config);
	void removeModbusClient(int idx);
	JZModbusClient* modbusClient(int idx);

protected:
	QList<JZModbusClient*> m_modbusClient;
	QList<JZModbusServer*> m_modbusServer;
};
void JZCommInit(JZCommManager* inst, const QByteArray& buffer);

#endif // !JZ_COMM_MANAGER_H_
