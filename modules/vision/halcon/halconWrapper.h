#ifndef JZHalconWrapper_H
#define JZHalconWrapper_H

#include <QVariant>

#include <QString>
#include <QDebug>
#include "../JZModelEngine.h"

// Qt风格的DLL导入导出宏
#ifdef JZHALCONDLL_EXPORTS
#define JZHALCONDLL_API Q_DECL_EXPORT
#else
#define JZHALCONDLL_API Q_DECL_IMPORT
#endif

class JZHalconWrapper
{
public:
    bool call(QString function,const QVariantList &in,const QVariantList &out); 
};

extern "C" JZHALCONDLL_API JZHalconWrapper *createJZHalconWrapper();


#endif