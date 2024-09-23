#ifndef JZ_MODULE_MODBUS_H_
#define JZ_MODULE_MODBUS_H_

#include "JZModule.h"

class JZModuleOpencv: public QObject, public JZModule
{
    Q_OBJECT
    Q_INTERFACES(JZModule)
    Q_PLUGIN_METADATA(IID JZModulePluginInterface_iid)

public:
    JZModuleOpencv();
    virtual ~JZModuleOpencv();
    
    virtual void regist() override;
    virtual void unregist() override;
};


#endif