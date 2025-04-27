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
    JZNodeModbusRead();
    ~JZNodeModbusRead();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

    void setCommIndex(int comm);

    void setFunction(int function);
    int function();

    void setAddr(int addr);
    int addr();
    
    void setReadType(QString type);
    QString readType();

protected:
    virtual bool updateNode(QString& error) override;
    int readTypeSize();

    int m_commIndex;
    int m_function;
    QString m_readType;    
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
    int m_commIndex;
    int m_function;
    QString m_readType;
    int m_bitOrder;

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