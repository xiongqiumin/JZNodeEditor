#ifndef JZ_MODULE_MODBUS_H_
#define JZ_MODULE_MODBUS_H_

#include "JZNodeFunction.h"
#include "JZModule.h"
#include "3rd/JZCommon/jzModbus/JZModbusParam.h"
#include "../JZModuleDefine.h"

enum 
{
    Node_modbusConfig = Module_ModbusType,    
};

class JZNodeModbusConfig : public JZNodeFunctionCustom
{
public:
    JZNodeModbusConfig();
    ~JZNodeModbusConfig();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    virtual void initFunction() override;

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

protected:    
    QString className();
    JZModbusConfig m_config;
};

class JZModuleModbus: public JZModule
{        
    
public:
    JZModuleModbus();
    virtual ~JZModuleModbus();
    
    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;
};












#endif