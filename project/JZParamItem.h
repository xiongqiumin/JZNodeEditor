#ifndef JZSCRIPT_PARAM_DEFINE_FILE_H_
#define JZSCRIPT_PARAM_DEFINE_FILE_H_

#include "JZProjectItem.h"
#include "JZNode.h"
#include <QMap>
#include <QVariant>

class JZParamItem : public JZProjectItem
{
public:
    JZParamItem();
    virtual ~JZParamItem();

    QByteArray toBuffer();
    bool fromBuffer(const QByteArray &object);

    void addVariable(QString name,QString type, const QString &v = QString());
    void addVariable(QString name,int type, const QString &v = QString());
    void removeVariable(QString name);
    void setVariable(QString name, JZParamDefine define);
    const JZParamDefine *variable(QString name) const;

    QStringList variableList();

    void addBind(JZNodeParamBind widget);
    void removeBind(QString name);    
    JZNodeParamBind *bindVariable(QString name);
    QMap<QString, JZNodeParamBind> bindVariables();
    
protected:        
    QMap<QString, JZParamDefine> m_variables;
    QMap<QString, JZNodeParamBind> m_binds;
};


#endif
