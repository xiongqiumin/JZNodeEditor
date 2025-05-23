#ifndef JZ_COMM_MODBUS_CLIENT_H_
#define JZ_COMM_MODBUS_CLIENT_H_

#include "../JZComm.h"
#include "3rd/JZCommon/jzModbus/JZModbusClient.h"

//JZCommModbusConfig
class JZCommModbusClientConfig : public JZCommConfig
{
public:
    JZCommModbusClientConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    JZModbusConnetInfo conn;
    QDataStream::ByteOrder bitOrder;
};


//JZCommModbusClient
class JZCommModbusClient : public JZCommObject
{
public:
    JZCommModbusClient(QObject* parent = nullptr);

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;
};


#endif