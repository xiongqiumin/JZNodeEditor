#ifndef JZSCRIPT_PARAM_DEFINE_FILE_H_
#define JZSCRIPT_PARAM_DEFINE_FILE_H_

#include "JZProjectItem.h"
#include "JZNode.h"
#include <QMap>
#include <QVariant>

class JZScriptParamDefineFile : public JZProjectItem
{
public:
    JZScriptParamDefineFile();
    virtual ~JZScriptParamDefineFile();

    void addVariable(QString name,int type,QVariant = QVariant());
    void removeVariable(QString name);
    JZParamDefine *getVariable(QString name);
    QStringList variableList();

protected:
    QMap<QString,JZParamDefine> m_variables;
};


#endif
