#ifndef JZ_MODULE_MODBUS_H_
#define JZ_MODULE_MODBUS_H_

#include "JZNodeFunction.h"
#include "JZModule.h"
#include "jzmodbus/JZModbusParam.h"

enum {
    Node_modbusConfig = 1000,    
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
    virtual JZNodePinWidget *createWidget(int id);
    QString className();

    JZModbusConfig m_config;
};

class JZModuleModbus: public QObject, public JZModule
{
    Q_OBJECT
    Q_INTERFACES(JZModule)
    Q_PLUGIN_METADATA(IID JZModulePluginInterface_iid)
    
public:
    JZModuleModbus();
    virtual ~JZModuleModbus();
    
    virtual void regist(JZScriptEnvironment *env) override;
    virtual void unregist(JZScriptEnvironment *env) override;
};












#endif