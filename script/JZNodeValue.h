#ifndef JZNODE_VALUE_H_
#define JZNODE_VALUE_H_

#include "JZNode.h"

//JZNodeLiteral
class JZNodeLiteral : public JZNode
{
public:
    JZNodeLiteral();
    ~JZNodeLiteral();        

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;    

    int dataType() const;
    void setDataType(int type);

    QVariant literal() const;
    void setLiteral(QVariant value);    

protected:

};

//JZNodeEnum
class JZNodeEnum : public JZNode
{
public:
    JZNodeEnum();
    ~JZNodeEnum();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

    void setEnum(int id);
    void setEnumValue(int value);

protected:
    int m_enumId;
};

//JZNodeParamFunction
class JZNodeParamFunction : public JZNode
{
public:
    JZNodeParamFunction();
    ~JZNodeParamFunction();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

    void setFucntion(QString name);    

protected:

};

//JZNodeDisplay
class JZNodeDisplay : public JZNode
{
public:
    JZNodeDisplay();
    ~JZNodeDisplay();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
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

//JZNodeCreateFromString
class JZNodeCreateFromString : public JZNode
{
public:
    JZNodeCreateFromString();
    ~JZNodeCreateFromString();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    virtual void pinChanged(int id) override;

    void setClassName(const QString &name);
    QString className() const;

    void setContext(const QString &text);
    QString context() const;
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

    virtual void drag(const QVariant &value);

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

    void setValue(const QString &name);
    QString value() const;

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
    
    void setValue(const QString &name);
    QString value() const;

    virtual void drag(const QVariant &value) override;

protected:    

};

//JZNodeAbstractMember
class JZNodeAbstractMember : public JZNode
{
public:
    void setMember(QString className,QStringList params);
    QString className();
    QStringList members();    
};

//JZNodeMemberParam
class JZNodeMemberParam : public JZNodeAbstractMember
{
public:
    JZNodeMemberParam();
    ~JZNodeMemberParam();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

//JZNodeSetMemberParam
class JZNodeSetMemberParam : public JZNodeAbstractMember
{
public:
    JZNodeSetMemberParam();
    ~JZNodeSetMemberParam();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;    
};

//JZNodeSetMemberParamData
class JZNodeSetMemberParamData : public JZNodeAbstractMember
{
public:
    JZNodeSetMemberParamData();
    ~JZNodeSetMemberParamData();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

//JZNodeClone
class JZNodeClone : public JZNode
{
public:
    JZNodeClone();
    ~JZNodeClone();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

//JZNodeSwap
class JZNodeSwap : public JZNode
{
public:
    JZNodeSwap();
    ~JZNodeSwap();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

#endif
