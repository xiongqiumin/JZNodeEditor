#include "JZNodeObject.h"
#include <QMetaObject>

//JZNodeObjectDefine
JZNodeObjectDefine::JZNodeObjectDefine()
{
    super = nullptr;
}


//JZNodeObject
JZNodeObject::JZNodeObject()
{
    m_pointer = nullptr;
}

JZNodeObject::~JZNodeObject()
{
}

QVariant JZNodeObject::param(QString name)
{
    return 0;
}

void JZNodeObject::setParam(QString name,QVariant value)
{
}

FunctionDefine *JZNodeObject::function(QString function)
{
    return nullptr;
}

//JZNodeObjectManager
JZNodeObjectManager::JZNodeObjectManager()
{        
}

void JZNodeObjectManager::registWidgets()
{
    
}