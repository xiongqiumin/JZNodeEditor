#ifndef JZEVENT_H_
#define JZEVENT_H_

#include <QEvent>
#include <QVariantList>
#include "JZNodeFunctionDefine.h"

class JZNodeObject;
class JZEvent: public QEvent
{
public:    
    static int Event;

    JZEvent();
    virtual ~JZEvent();
    
    int eventType;    
    JZNodeObject *sender;
    QVariantList params;    
};


#endif
