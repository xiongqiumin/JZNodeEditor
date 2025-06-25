#ifndef JZ_COMM_MODBUS_SERVER_H_
#define JZ_COMM_MODBUS_SERVER_H_

#include "../JZComm.h"
#include "3rd/JZCommon/jzModbus/JZModbusServer.h"

//JZCommModbusRtuServerConfig
class JZCommModbusRtuServerConfig : public JZCommConfig
{
public:
    JZCommModbusRtuServerConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    JZModbusConnetInfo conn;
    QDataStream::ByteOrder bitOrder;
};

//JZCommModbusTcpServerConfig
class JZCommModbusTcpServerConfig : public JZCommConfig
{
public:
    JZCommModbusTcpServerConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    JZModbusConnetInfo conn;
    QDataStream::ByteOrder bitOrder;
};


//JZCommModbusServer
class JZCommModbusServer : public JZCommObject
{
    Q_OBJECT

public:
    JZCommModbusServer(QObject* parent = nullptr);
    ~JZCommModbusServer();

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;

    JZModbusServer *server();

public:
    JZModbusServer *m_server;
};


#endif // ! JZ_COMM_MODBUS_SERVER_H_
