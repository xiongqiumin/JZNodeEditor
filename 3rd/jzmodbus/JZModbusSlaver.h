#ifndef JZ_MODBUS_SLAVE_H_
#define JZ_MODBUS_SLAVE_H_

#include <QObject>
#include "JZModbusParam.h"
#include "JZModbusServer.h"

class JZModbusSlaver : public QObject
{
    Q_OBJECT

public:
    JZModbusSlaver(QObject *parent = nullptr);
    ~JZModbusSlaver();

    void setSlave(int slave);

    void initRtu(QString com, int baud, QSerialPort::DataBits, QSerialPort::StopBits, QSerialPort::Parity);
    void initTcp(int port);
    bool startServer();
    void stopServer();

    JZModbusParamMap *map();
    void initMapping();

    void writeParam(int addr, QVariant value);
    QVariant readParam(int addr);

signals:
    void sigParamChanged(int addr);

protected slots:
    void onMappingChanged(int addr, int nb);

protected:
    JZModbusParamMap m_map;
    JZModbusServer *m_server;

};
void modbusSlaverSetConfig(JZModbusSlaver *slaver, const JZModbusConfig *cfg);

#endif