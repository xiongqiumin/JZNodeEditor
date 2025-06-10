#ifndef HALCONWRAPPER_H
#define HALCONWRAPPER_H

#include <QVariant>

class HalconWrapper
{
public:
    bool call(QString function,const QVariantList &in,const QVariantList &out); 
};

HalconWrapper *createHalconWrapper();


#endif