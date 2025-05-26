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

//JZNodeFormatImpl
class JZNodeFormatImpl : public JZNode
{
public:
    JZNodeFormatImpl();
    ~JZNodeFormatImpl();

    void setFormat(QString format);
    QString format();

    virtual bool compiler(JZNodeCompiler* compiler, QString& error) override;

protected:
    virtual bool updateNode(QString& error) override;

    bool m_isText;
    int m_argIndex;
    QString m_function;
};

//JZNodeFormat
class JZNodeFormat : public JZNodeFormatImpl
{
public:
    JZNodeFormat();
    ~JZNodeFormat();
};

//JZNodeFormatBin
class JZNodeFormatBin : public JZNodeFormatImpl
{
public:
    JZNodeFormatBin();
    ~JZNodeFormatBin();
};

//JZNodePrint
class JZNodePrint : public JZNodeFormatImpl
{
public:
    JZNodePrint();
    ~JZNodePrint();
};


//JZNodeLog
class JZNodeLog : public JZNodeFormatImpl
{
public:
    JZNodeLog();
    ~JZNodeLog();
};


//JZNodeDisplay
class JZNodeDisplay : public JZNode
{
public:
    JZNodeDisplay();
    ~JZNodeDisplay();

    void addInput();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
protected:

};

//JZNodeCreateObject
class JZNodeCreateObject : public JZNode
{
public:
    JZNodeCreateObject();
    ~JZNodeCreateObject();

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

    void setValue(const QString & value);
    QString value() const;

protected:        

};

//JZNodeMemberParam
class JZNodeMemberParam : public JZNode
{
public:
    JZNodeMemberParam();
    ~JZNodeMemberParam();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    bool update(QString &error);

    QString m_calssName;
};

//JZNodeSetMemberParam
class JZNodeSetMemberParam : public JZNode
{
public:
    JZNodeSetMemberParam();
    ~JZNodeSetMemberParam();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    bool update(QString &error);
    
    QString m_calssName;
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
