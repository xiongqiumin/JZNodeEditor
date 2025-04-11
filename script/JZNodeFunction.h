#ifndef JZNODE_FUNCTION_H_
#define JZNODE_FUNCTION_H_

#include "JZNode.h"
#include "JZNodeFunctionDefine.h"

class JZNodeFunction : public JZNode
{
public:
    JZNodeFunction();
    virtual ~JZNodeFunction();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
    
    void setFunction(const JZFunctionDefine *define);
    QString function() const;
    JZFunctionDefine functionDefine();

    void setVariable(const QString& name);  //在当前作用域的变量名，用于成员函数调用
    QString variable() const;

    void setDirectCall(bool flag);
    bool isDirectCall();

protected:
    virtual bool update(QString &error) override;    
    void updateName();
    bool isMemberCall();

    bool m_directCall;
    QString m_functionName;
};

class JZNodeFunctionCustom : public JZNode
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
