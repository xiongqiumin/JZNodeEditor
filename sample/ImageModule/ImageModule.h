#ifndef IMAGE_MODULE_H_
#define IMAGE_MODULE_H_

#include "JZModule.h"

class ModuleImageSample : public JZModule
{
public:    
    ModuleImageSample();
    ~ModuleImageSample();

    virtual void regist() override;
    virtual void unregist() override;

protected:
    
};


#endif