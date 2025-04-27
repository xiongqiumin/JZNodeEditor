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

    void setConfig(JZCommConfig config);
    JZCommConfig config();

    bool compiler(JZNodeCompiler* compiler, QString& error);

    JZCommConfig m_config;
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

    void setClient(QString comm);
    QString client();

    void setAddr(int addr);
    int addr();

    void setFunction(int function);
    int function();
    
    void setDataType(QString type);
    QString dataType();

protected:
    virtual bool updateNode(QString& error) override;

    QString m_modbus;
    int m_function;
    QString m_dataType;
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

    void setClient(QString comm);
    QString client();

    void setAddr(int addr);
    int addr();

    void setFunction(int function);
    int function();

    void setDataType(QString type);
    QString dataType();

protected:
    bool updateNode(QString& error);

    QString m_modbus;
    int m_function;
    QString m_dataType;

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