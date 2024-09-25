#ifndef JZNODE_FUNCTION_H_
#define JZNODE_FUNCTION_H_

#include "JZNode.h"
#include "JZNodeFunctionDefine.h"

class JZCORE_EXPORT JZNodeFunction : public JZNode
{
public:
    JZNodeFunction();
    virtual ~JZNodeFunction();

    void setVariable(const QString &name);
    QString variable() const;

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
    
    void setFunction(const JZFunctionDefine *define);
    QString function() const;
    JZFunctionDefine functionDefine();

protected:
    virtual bool update(QString &error) override;    
    void updateName();
    bool isMemberCall();

    QString m_functionName;
};

class JZCORE_EXPORT JZNodeFunctionCustom : public JZNode
{
public:
    JZNodeFunctionCustom();
    ~JZNodeFunctionCustom();

public:
    void setFunction(const QString &name);    
    QString function() const;

protected:
    virtual void initFunction() = 0;

    QString m_functionName;
};

#endif
