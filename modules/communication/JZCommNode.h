#ifndef JZ_COMM_NODE_H_
#define JZ_COMM_NODE_H_

#include "JZNode.h"
#include "../JZModuleDefine.h"
#include "JZCommManager.h"

enum {
    Node_CommInit = Module_CommNode,
    Node_ModbusRead, 
    Node_ModbusWrite,
    Node_TcpClientRead,
    Node_TcpClientWrite,
    Node_UdpRead,
    Node_UdpWrite,
    Node_SerialRead,
    Node_SerialWrite,

    Node_modbusWatch,
    Node_modbusConfig,
};


JZModbusClient* JZModbusGet(JZCommManager* manager, int conn);

//JZNodeCommInit
class JZNodeCommInit : public JZNode
{
public:
    JZNodeCommInit();
    ~JZNodeCommInit();

    bool compiler(JZNodeCompiler* compiler, QString& error);
};

//JZNodeModbusRead
class JZNodeModbusRead : public JZNode
{
public:
    enum {
        readBit,
        readBits,
        readInputBit,
        readInputBits,
        readInputRegister,
        readInputRegisters,
        readRegister,
        readRegisters,
    };

    JZNodeModbusRead();
    ~JZNodeModbusRead();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

    void setCommIndex(int comm);
    int readComm();

    void setFunction(int function);
    int function();

    void setAddr(int addr);
    int addr();

    void setCount(int count);
    int count();

    void setReadType(QString type);
    QString readType();

    void setBitOrder(int order);
    int bitOrder();

protected:
    virtual bool updateNode(QString& error) override;
    int readTypeSize();

    int m_commIndex;
    int m_function;
    QString m_readType;
    int m_bitOrder;
};

//JZNodeModbusWrite
class JZNodeModbusWrite : public JZNode
{
public:
    JZNodeModbusWrite();
    ~JZNodeModbusWrite();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:
    QString addr;

};

//JZNodeTcpClientRead
class JZNodeTcpClientRead : public JZNode
{
public:
    JZNodeTcpClientRead();
    ~JZNodeTcpClientRead();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:
};

//JZNodeTcpClientWrite
class JZNodeTcpClientWrite : public JZNode
{
public:
    JZNodeTcpClientWrite();
    ~JZNodeTcpClientWrite();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:
    QString addr;

};

//JZNodeUdpRead
class JZNodeUdpRead : public JZNode
{
public:
    JZNodeUdpRead();
    ~JZNodeUdpRead();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:
    QString addr;

};

//JZNodeUdpWrite
class JZNodeUdpWrite : public JZNode
{
public:
    JZNodeUdpWrite();
    ~JZNodeUdpWrite();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:
    QString addr;

};

//JZNodeSerialRead
class JZNodeSerialRead : public JZNode
{
public:
    JZNodeSerialRead();
    ~JZNodeSerialRead();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:
    QString addr;

};

//JZNodeSerialWrite
class JZNodeSerialWrite : public JZNode
{
public:
    JZNodeSerialWrite();
    ~JZNodeSerialWrite();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:
    QString addr;

};

#endif