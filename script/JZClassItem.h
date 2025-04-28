#ifndef JZNODE_CLASS_FILE_H_
#define JZNODE_CLASS_FILE_H_

#include "JZNode.h"
#include "JZProjectItem.h"
#include "JZScriptItem.h"
#include "JZNodeObject.h"
#include "JZUiItem.h"

class JZParamItem;
class JZScriptClassItem : public JZProjectItem
{
public:
    JZScriptClassItem();
    virtual ~JZScriptClassItem();    

    void setClass(QString className, QString super);
    QString className() const;

    int classType();


    QString superClass() const;

    JZNodeObjectDefine objectDefine(); //只应该在重新注册类时使用
    
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

    JZScriptItem* addFlow(QString name);
    void removeFlow(QString name);
    JZScriptItem* flow(QString name);
    QStringList flowList();

    JZUiItem *ui();
    bool hasUi();
    void addUi(JZUiItem *item);
    void removeUi();
    QList<JZParamDefine> uiWidgets();

protected:
    virtual void saveToStream(QDataStream &s) const override;
    virtual bool loadFromStream(QDataStream &s) override;    
    QString m_super;  

    JZParamDefine m_this;
};

#endif
