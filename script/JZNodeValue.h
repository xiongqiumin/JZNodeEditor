#ifndef JZNODE_DATA_SOURCE_H_
#define JZNODE_DATA_SOURCE_H_

#include "JZNode.h"

//JZNodeValue
class JZNodeLiteral : public JZNode
{
public:
    JZNodeLiteral();
    ~JZNodeLiteral();        

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;    

    int dataType();
    void setDataType(int type);

    QVariant literal() const;
    void setLiteral(QVariant value);    

protected:

};

//JZNodePrint
class JZNodePrint : public JZNode
{
public:
    JZNodePrint();
    ~JZNodePrint();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;   
protected:

};

//JZNodeCreate
class JZNodeCreate : public JZNode
{
public:
    JZNodeCreate();
    ~JZNodeCreate();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual void pinChanged(int id) override;

    void setClassName(const QString &name);
    QString className() const;
};

//JZNodeParamThis
class JZNodeThis : public JZNode
{
public:
    JZNodeThis();
    ~JZNodeThis();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual void fileInitialized() override;
};

//JZNodeParam
class JZNodeParam : public JZNode
{
public:
    JZNodeParam();
    ~JZNodeParam();        

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual void pinChanged(int id) override;

    void setVariable(const QString &name);
    QString variable() const;
    virtual void drag(const QVariant &value) override;

protected:

};

//JZNodeSetParam
class JZNodeSetParam : public JZNode
{
public:
    JZNodeSetParam();
    ~JZNodeSetParam();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual void pinChanged(int id) override;
    
    void setVariable(const QString &name);
    QString variable() const;
    virtual void drag(const QVariant &value) override;

protected:        

};


//JZNodeSetParamDataFlow
class JZNodeSetParamDataFlow : public JZNode
{
public:
    JZNodeSetParamDataFlow();
    ~JZNodeSetParamDataFlow();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual void pinChanged(int id) override;
    
    void setVariable(const QString &name);
    QString variable() const;
    virtual void drag(const QVariant &value) override;

protected:    

};

#endif
