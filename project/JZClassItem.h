﻿#ifndef JZNODE_CLASS_FILE_H_
#define JZNODE_CLASS_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZScriptItem.h"
#include "JZNodeObject.h"

class JZParamItem;
class JZCORE_EXPORT JZScriptClassItem : public JZProjectItem
{
public:
    JZScriptClassItem();
    virtual ~JZScriptClassItem();    

    void setClass(QString className, QString super);
    QString className() const;

    int classType();
    void setClassType(int classId);

    QString superClass() const;

    JZNodeObjectDefine objectDefine();
    
    JZParamItem *paramFile();
    bool addMemberVariable(QString name,int dataType,const QString &v = QString());
    bool addMemberVariable(QString name,QString dataType, const QString &v = QString());
    bool addMemberVariable(JZParamDefine param);
    void removeMemberVariable(QString name);    
    QStringList memberVariableList(bool hasUi);
    const JZParamDefine *memberVariable(QString name, bool hasUi);
    const JZParamDefine *memberThis();

    JZScriptItem *addMemberFunction(JZFunctionDefine func);
    void removeMemberFunction(QString func);
    JZScriptItem *memberFunction(QString func);    
    QStringList memberFunctionList();

    QString uiFile() const;
    void setUiFile(QString file);
    QList<JZParamDefine> uiWidgets();

protected:
    virtual void saveToStream(QDataStream &s) const override;
    virtual bool loadFromStream(QDataStream &s) override;

    int m_classId;
    QString m_super;
    QString m_uiFile;        

    JZParamDefine m_this;
};

#endif
