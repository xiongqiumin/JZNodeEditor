#ifndef JZNODE_VALUE_H_
#define JZNODE_VALUE_H_

#include "JZNode.h"
#include "JZNodeObject.h"

//JZNodeLiteral
class JZNodeLiteral : public JZNode
{
public:
    JZNodeLiteral();
    ~JZNodeLiteral();        

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;    

    int dataType() const;
    void setDataType(int type);    

    QString literal() const;
    void setLiteral(const QString &value);    

protected:

};

//JZNodeEnum
class JZNodeEnum : public JZNode
{
public:
    JZNodeEnum();
    ~JZNodeEnum();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

    void setEnum(const JZNodeEnumDefine *def);
    void setKey(QString text);    
};

//JZNodeFlag
class JZNodeFlag : public JZNode
{
public:
    JZNodeFlag();
    ~JZNodeFlag();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

    void setFlag(const JZNodeEnumDefine *def);
    void setKey(QString value);    
};

//JZNodeConvert
class JZNodeConvert : public JZNode
{
public:
    JZNodeConvert();
    ~JZNodeConvert();

    void setOutputType(int type);

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:
    virtual bool updateNode(QString &error) override;
};

//JZNodeFunctionPointer
class JZNodeFunctionPointer : public JZNode
{
public:
    JZNodeFunctionPointer();
    ~JZNodeFunctionPointer();

    void setFucntion(QString name);
    QString fucntion();
    
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

\
//JZNodeFormat
class JZNodeFormat : public JZNode
{
public:
    JZNodeFormat();
    ~JZNodeFormat();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;
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

//JZNodeCreate
class JZNodeCreate : public JZNode
{
public:
    JZNodeCreate();
    ~JZNodeCreate();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;    
    virtual bool updateNode(QString &error) override;

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
    virtual bool updateNode(QString &error) override;

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
    virtual bool updateNode(QString &error) override;
};

//JZNodeParam
class JZNodeParam : public JZNode
{
public:
    JZNodeParam();
    ~JZNodeParam();        

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual bool updateNode(QString &error) override;

    void setVariable(const QString &name);
    QString variable() const;

protected:

};

//JZNodeSetParam
class JZNodeSetParam : public JZNode
{
public:
    JZNodeSetParam();
    ~JZNodeSetParam();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual bool updateNode(QString &error) override;
    
    void setVariable(const QString &name);
    QString variable() const;

    void setValue(const QString &name);
    QString value() const;

protected:        

};

//JZNodeAbstractMember
class JZNodeAbstractMember : public JZNode
{
public:
    JZNodeAbstractMember();
    ~JZNodeAbstractMember();

    QString className();
    void setClassName(QString className);

    QString member();
    void setMember(QString params);    

protected:
    bool update(QString &error);
    int m_classType;
    int m_memberId;
    QString m_memberType;
};

//JZNodeMemberParam
class JZNodeMemberParam : public JZNodeAbstractMember
{
public:
    JZNodeMemberParam();
    ~JZNodeMemberParam();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    bool update(QString &error);
};

//JZNodeSetMemberParam
class JZNodeSetMemberParam : public JZNodeAbstractMember
{
public:
    JZNodeSetMemberParam();
    ~JZNodeSetMemberParam();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    bool update(QString &error);
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
