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
    
    void setFunction(QString fullName);
    void setFunction(const JZFunctionDefine *define);
    QString function() const;    

    void setVariable(const QString& name);  //在当前作用域的变量名，用于成员函数调用
    QString variable() const;

    void setDirectCall(bool flag);
    bool isDirectCall();
    bool isMemberCall();

protected:
    virtual bool updateNode(QString &error) override;
    bool updateFunctionDefine(const JZFunctionDefine *define, QString &error);

    bool m_directCall;
    QString m_functionName;
};

#endif
