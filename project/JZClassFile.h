#ifndef JZNODE_CLASS_FILE_H_
#define JZNODE_CLASS_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZScriptFile.h"
#include "JZNodeObject.h"

class JZParamFile;
class JZScriptClassFile : public JZProjectItem
{
public:
    JZScriptClassFile();
    virtual ~JZScriptClassFile();    

    virtual void saveToStream(QDataStream &s);
    virtual void loadFromStream(QDataStream &s);    

    void setClass(QString className, QString super);
    QString className() const;

    int classType() const;
    void setClassType(int classId);

    QString superClass() const;

    JZNodeObjectDefine objectDefine();
    
    JZParamFile *getParamFile();
    bool addMemberVariable(QString name,int dataType,const QVariant &v = QVariant());
    void removeMemberVariable(QString name);
    JZParamDefine *memberVariableInfo(QString name);

    JZScriptFile *addMemberFunction(FunctionDefine func);    
    void removeMemberFunction(QString func);
    JZScriptFile *getMemberFunction(QString func);    

    QList<JZParamDefine> uiWidgets();

protected:       
    QString m_className;
    QString m_super;
    int m_classId;
};

#endif
