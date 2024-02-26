﻿#ifndef JZNODE_FUNCTION_H_
#define JZNODE_FUNCTION_H_

#include "JZNode.h"
#include "JZNodeFunctionDefine.h"

class JZNodeFunctionStart : public JZNode
{
public:    
    JZNodeFunctionStart();
    virtual ~JZNodeFunctionStart();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;    

protected:
    
};

class JZNodeFunction : public JZNode
{
public:
    JZNodeFunction();
    virtual ~JZNodeFunction();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    virtual void saveToStream(JZProjectStream &s) const override;
    virtual void loadFromStream(JZProjectStream &s) override;

    void setFunction(const FunctionDefine *define);
    QString function() const;

protected:
    bool pinClicked(int id);

    QString m_functionName;
};

#endif
