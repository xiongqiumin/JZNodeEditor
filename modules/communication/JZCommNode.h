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

    void setConfig(JZCommManagerConfig config);
    JZCommManagerConfig config();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

    JZCommManagerConfig m_config;
};

//JZCommNode
class JZCommNode : public JZNode
{
public:
    JZCommNode();
    ~JZCommNode();

    void setName(QString comm);
    QString name();

protected:
    bool checkCommManager(JZNodeCompiler *c, QString &error);
};

//JZNodeModbusRead
class JZNodeModbusRead : public JZCommNode
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
class JZNodeModbusWrite : public JZCommNode
{
public:
    JZNodeModbusWrite();
    ~JZNodeModbusWrite();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;    

    void setAddr(int addr);
    int addr();

    void setValue(QString value);
    QString value();

    void setFunction(int function);
    int function();

    void setDataType(QString type);
    QString dataType();

protected:
    bool updateNode(QString& error);

    int m_function;
    QString m_dataType;
};

//JZNodeTcpClientRead
class JZNodeTcpClientRead : public JZCommNode
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
class JZNodeTcpClientWrite : public JZCommNode
{
public:
    JZNodeTcpClientWrite();
    ~JZNodeTcpClientWrite();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:

};

//JZNodeUdpRead
class JZNodeUdpRead : public JZCommNode
{
public:
    JZNodeUdpRead();
    ~JZNodeUdpRead();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:

};

//JZNodeUdpWrite
class JZNodeUdpWrite : public JZCommNode
{
public:
    JZNodeUdpWrite();
    ~JZNodeUdpWrite();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:

};

//JZNodeSerialRead
class JZNodeSerialRead : public JZCommNode
{
public:
    JZNodeSerialRead();
    ~JZNodeSerialRead();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:

};

//JZNodeSerialWrite
class JZNodeSerialWrite : public JZCommNode
{
public:
    JZNodeSerialWrite();
    ~JZNodeSerialWrite();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:

};

#endif