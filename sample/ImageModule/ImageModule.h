#ifndef IMAGE_MODULE_H_
#define IMAGE_MODULE_H_

#include "JZModule.h"

class ImageModule : public JZModule
{
public:
    static void init();

    ImageModule();
    ~ImageModule();

    virtual void regist() override;
    virtual void unregist() override;

protected:
    QStringList m_functions;
};


#endif