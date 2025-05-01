#ifndef JZ_MODULE_MODBUS_H_
#define JZ_MODULE_MODBUS_H_

#include "JZNodeFunction.h"
#include "3rd/JZCommon/jzModbus/JZModbusParam.h"
#include "3rd/JZCommon/jzModbus/JZModbusMaster.h"
#include "3rd/JZCommon/jzModbus/JZModbusSlaver.h"
#include "JZNodeEvent.h"

class JZNodeModbusConfig : public JZNode
{
public:
    JZNodeModbusConfig();
    ~JZNodeModbusConfig();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:
    JZModbusConfig m_config;
};

class JZNodeModbusWatchEvent : public JZNodeSignalEvent
{
public:
    JZNodeModbusWatchEvent();
    ~JZNodeModbusWatchEvent();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

    virtual void saveToStream(QDataStream& s) const override;
    virtual void loadFromStream(QDataStream& s) override;

protected:
};
void JZNodeModbusWatchEventInit();

#endif