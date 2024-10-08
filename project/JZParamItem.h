﻿#ifndef JZSCRIPT_PARAM_DEFINE_FILE_H_
#define JZSCRIPT_PARAM_DEFINE_FILE_H_

#include "JZProjectItem.h"
#include "JZNode.h"
#include <QMap>
#include <QVariant>

class JZCORE_EXPORT JZParamItem : public JZProjectItem
{
public:
    JZParamItem();
    virtual ~JZParamItem();

    virtual void saveToStream(QDataStream &s) const override;
    virtual bool loadFromStream(QDataStream &s) override;

    void addVariable(QString name,QString type, const QString &v = QString());
    void addVariable(QString name,int type, const QString &v = QString());
    void addVariable(JZParamDefine define);
    void removeVariable(QString name);
    void setVariable(QString name, JZParamDefine define);
    const JZParamDefine *variable(QString name) const;
    QStringList variableList();
    
    void addBind(JZNodeParamBind info);
    void removeBind(QString name);    
    JZNodeParamBind *bindVariable(QString name);
    QStringList bindVariableList();    

protected:        
    QMap<QString, JZParamDefine> m_variables;
    QMap<QString, JZNodeParamBind> m_binds;
};

#endif
