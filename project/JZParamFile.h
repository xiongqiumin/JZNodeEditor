#ifndef JZSCRIPT_PARAM_DEFINE_FILE_H_
#define JZSCRIPT_PARAM_DEFINE_FILE_H_

#include "JZProjectItem.h"
#include "JZNode.h"
#include <QMap>
#include <QVariant>

class JZParamFile : public JZProjectItem
{
public:
    JZParamFile();
    virtual ~JZParamFile();

    virtual void saveToStream(QDataStream &s);
    virtual void loadFromStream(QDataStream &s);

    void addVariable(QString name,int type,QVariant = QVariant());
    void removeVariable(QString name);
    void renameVariable(QString oldName,QString newName);
    void setVariableType(QString name,int dataType);

    JZParamDefine *getVariable(QString name);
    QStringList variableList();
    const QMap<QString,JZParamDefine> &variables();

protected:
    QMap<QString,JZParamDefine> m_variables;
};


#endif
