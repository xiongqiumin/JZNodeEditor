#include "JZCommManager.h"

int JZCommManager::indexOfModbusClient(JZModbusConnetInfo config)
{
	for (int i = 0; i < m_modbusClient.size(); i++)
	{
		auto& conn = m_modbusClient[i]->connInfo();
		if (conn.modbusType != config.modbusType)
			continue;

		if (conn.modbusType == Modbus_rtuClient && conn.port == config.port)
		{
			return i;
		}
		else if (conn.modbusType == Modbus_rtuClient && conn.ip == config.ip && conn.port == config.port)
		{
			return i;
		}
	}
	return -1;
}

JZModbusClient* JZCommManager::newModbusClient(JZModbusConnetInfo config)
{
	JZModbusClient* client = new JZModbusClient(this);
	client->initConn(config);
	m_modbusClient.push_back(client);
	return client;
}

JZModbusClient* JZCommManager::modbusClient(int idx)
{
	return m_modbusClient[idx];
}

void JZCommManager::removeModbusClient(int idx)
{
	m_modbusClient[idx]->deleteLater();
	m_modbusClient.removeAt(idx);
}

void JZCommInit(JZCommManager* inst, const QByteArray& buffer)
{

}