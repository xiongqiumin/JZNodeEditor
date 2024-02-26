#ifndef JZNODE_CLASS_FILE_H_
#define JZNODE_CLASS_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZScriptItem.h"
#include "JZNodeObject.h"

class JZParamItem;
class JZScriptClassItem : public JZProjectItem
{
public:
    JZScriptClassItem();
    virtual ~JZScriptClassItem();    

    void setClass(QString className, QString super);
    QString className() const;

    int classType() const;
    void setClassType(int classId);

    QString superClass() const;

    JZNodeObjectDefine objectDefine();
    
    JZParamItem *getParamFile();
    bool addMemberVariable(QString name,int dataType,const QVariant &v = QVariant());
    void removeMemberVariable(QString name);
    JZParamDefine *memberVariableInfo(QString name);

    JZScriptItem *addMemberFunction(FunctionDefine func);    
    void removeMemberFunction(QString func);
    JZScriptItem *getMemberFunction(QString func);    

    QList<JZParamDefine> uiWidgets();

protected:       
    QString m_className;
    QString m_super;
    int m_classId;
};

#endif
