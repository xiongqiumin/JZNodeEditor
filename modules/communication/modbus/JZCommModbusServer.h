#ifndef JZ_COMM_MODBUS_SERVER_H_
#define JZ_COMM_MODBUS_SERVER_H_

#include "../JZComm.h"
#include "3rd/JZCommon/jzModbus/JZModbusServer.h"

//JZCommModbusConfig
class JZCommModbusServerConfig : public JZCommConfig
{
public:
    JZCommModbusServerConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    JZModbusConnetInfo conn;
    QDataStream::ByteOrder bitOrder;
};


//JZCommModbusServer
class JZCommModbusServer : public JZCommObject
{
public:
    JZCommModbusServer(QObject* parent = nullptr);

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;
};


#endif // ! JZ_COMM_MODBUS_SERVER_H_
