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

    QByteArray toBuffer();
    bool fromBuffer(const QByteArray &object);

    void setClass(QString className, QString super);
    QString className() const;

    int classType() const;
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
    int m_classId;
    QString m_super;
    QString m_uiFile;        

    JZParamDefine m_this;
};

#endif
