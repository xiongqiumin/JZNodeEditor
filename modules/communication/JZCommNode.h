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
    Node_TcpClientReadText,
    Node_TcpClientWriteText,
    Node_UdpRead,
    Node_UdpWrite,
    Node_SerialRead,
    Node_SerialWrite,
    Node_SerialReadText,
    Node_SerialWriteText,

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
    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
    QList<JZNodeIRParam> toParamId(QList<JZNodeIRParam> ir_list);
    
    QString m_function;
    QList<JZNodeIRParam> m_input;
    QList<JZNodeIRParam> m_output;
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
    int m_modbusFunc;
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

    int m_modbusFunc;
    QString m_dataType;
};

//JZNodeTcpClientRead
class JZNodeTcpClientRead : public JZCommNode
{
public:
    JZNodeTcpClientRead();
    ~JZNodeTcpClientRead();

protected:
};

//JZNodeTcpClientWrite
class JZNodeTcpClientWrite : public JZCommNode
{
public:
    JZNodeTcpClientWrite();
    ~JZNodeTcpClientWrite();

protected:

};

//JZNodeTcpClientReadText
class JZNodeTcpClientReadText : public JZCommNode
{
public:
    JZNodeTcpClientReadText();
    ~JZNodeTcpClientReadText();

protected:
};

//JZNodeTcpClientWriteText
class JZNodeTcpClientWriteText : public JZCommNode
{
public:
    JZNodeTcpClientWriteText();
    ~JZNodeTcpClientWriteText();

protected:

};

//JZNodeUdpRead
class JZNodeUdpRead : public JZCommNode
{
public:
    JZNodeUdpRead();
    ~JZNodeUdpRead();

protected:

};

//JZNodeUdpWrite
class JZNodeUdpWrite : public JZCommNode
{
public:
    JZNodeUdpWrite();
    ~JZNodeUdpWrite();

protected:

};

//JZNodeSerialRead
class JZNodeSerialRead : public JZCommNode
{
public:
    JZNodeSerialRead();
    ~JZNodeSerialRead();

protected:

};

//JZNodeSerialWrite
class JZNodeSerialWrite : public JZCommNode
{
public:
    JZNodeSerialWrite();
    ~JZNodeSerialWrite();

protected:

};


//JZNodeSerialReadText
class JZNodeSerialReadText : public JZCommNode
{
public:
    JZNodeSerialReadText();
    ~JZNodeSerialReadText();

protected:

};

//JZNodeSerialWriteText
class JZNodeSerialWriteText : public JZCommNode
{
public:
    JZNodeSerialWriteText();
    ~JZNodeSerialWriteText();

 
protected:

};

#endif