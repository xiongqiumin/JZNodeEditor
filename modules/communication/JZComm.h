#ifndef JZ_COMM_H_
#define JZ_COMM_H_

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include "../JZModuleConfigFactory.h"

enum {
    Comm_None,
    Comm_ModbusRtuClient,
    Comm_ModbusTcpClient,
    Comm_ModbusRtuServer,
    Comm_ModbusTcpServer,
    Comm_TcpClient,
    Comm_TcpServer,
    Comm_Udp,
    Comm_SerialPort,
};

//JZCommConfig
class JZCommConfig
{
public:
    JZCommConfig();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    int type;
    QString name;
};
typedef JZModuleConfigEnum<JZCommConfig> JZCommConfigEnum;

//JZCommObject
class JZCommObject : public QObject
{
    Q_OBJECT

public:	
    JZCommObject(QObject* parent = nullptr);
    virtual ~JZCommObject();

    void setConfig(JZCommConfigEnum config);

    virtual bool isOpen() = 0;
    virtual bool open() = 0;
    virtual void close() = 0;

protected:
    JZCommConfigEnum m_config;
};


#endif