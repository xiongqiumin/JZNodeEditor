﻿#ifndef JZNODE_UI_FILE_H_
#define JZNODE_UI_FILE_H_

#include "JZProjectItem.h"
#include "JZNode.h"
#include "JZNodeObject.h"

//JZUiFile
class JZUiFile : public JZProjectItem
{
public:
    JZUiFile();
    virtual ~JZUiFile();

    QString xml();
    void setXml(QString xml);

    QList<JZParamDefine> widgets();
    
protected:
    void updateDefine();

    QString m_xml;
    QList<JZParamDefine> m_widgets;
};



#endif
