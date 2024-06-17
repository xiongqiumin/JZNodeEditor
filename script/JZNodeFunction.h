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

    void setFunction(const QString &name);
    void setFunction(const JZFunctionDefine *define);
    QString function() const;

protected:
    QString m_functionName;
};

#endif
