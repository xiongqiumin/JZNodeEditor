#ifndef IMAGE_MODULE_H_
#define IMAGE_MODULE_H_

#include "JZModule.h"

class ModuleImageSample : public QObject, public JZModule
{
    Q_OBJECT
    Q_INTERFACES(JZModule)
    Q_PLUGIN_METADATA(IID JZModulePluginInterface_iid)

public:    
    ModuleImageSample();
    ~ModuleImageSample();

    virtual void regist() override;
    virtual void unregist() override;

protected:
    
};


#endif