#ifndef JZ_COMM_MODBUS_CLIENT_H_
#define JZ_COMM_MODBUS_CLIENT_H_

#include "../JZComm.h"
#include "3rd/JZCommon/jzModbus/JZModbusClient.h"

//JZCommModbusRtuClientConfig
class JZCommModbusRtuClientConfig : public JZCommConfig
{
public:
    JZCommModbusRtuClientConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    JZModbusConnetInfo conn;
    QDataStream::ByteOrder bitOrder;
};


//JZCommModbusTcpClientConfig
class JZCommModbusTcpClientConfig : public JZCommConfig
{
public:
    JZCommModbusTcpClientConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    JZModbusConnetInfo conn;
    QDataStream::ByteOrder bitOrder;
};

//JZCommModbusClient
class JZCommModbusClient : public JZCommObject
{
    Q_OBJECT

public:
    JZCommModbusClient(QObject* parent = nullptr);
    ~JZCommModbusClient();

    virtual bool isOpen() override;
    virtual bool open() override;
    virtual void close() override;
    JZModbusClient *client();

    bool readBits(int addr, int nb, QVector<uint8_t>& dest);
    bool readInputBits(int addr, int nb, QVector<uint8_t>& dest);
    bool readRegisters(int addr, int nb, QVector<uint16_t>& dest);
    bool readInputRegisters(int addr, int nb, QVector<uint16_t>& dest);

    bool writeBit(int coil_addr, int status);
    bool writeRegisters(int addr, const QVector<uint16_t>& dest);

protected slots:
    void onModbusReplay(const JZModebusReply& reply);
    void onModbusResult(bool flag);

protected:
    enum
    {
        None,
        Connecting,
        ConnectSuccessed,
        ConnectFailed,
    };

    bool waitReplay();

    bool m_waitReplay;
    int m_waitConnect;
    JZModebusReply m_reply;
    JZModbusClient *m_client;
};


#endif