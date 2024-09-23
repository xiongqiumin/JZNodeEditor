#ifndef JZNODE_VALUE_H_
#define JZNODE_VALUE_H_

#include "JZNode.h"

//JZNodeLiteral
class JZCORE_EXPORT JZNodeLiteral : public JZNode
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
class JZCORE_EXPORT JZNodeEnum : public JZNode
{
public:
    JZNodeEnum();
    ~JZNodeEnum();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

    void setEnum(QString text);
    void setKey(QString text);
    void setValue(int value);
};

//JZNodeFlag
class JZCORE_EXPORT JZNodeFlag : public JZNode
{
public:
    JZNodeFlag();
    ~JZNodeFlag();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

    void setFlag(QString flag);
    void setKey(QString value);
    void setValue(int value);
};

//JZNodeConvert
class JZCORE_EXPORT JZNodeConvert : public JZNode
{
public:
    JZNodeConvert();
    ~JZNodeConvert();

    void setOutputType(int type);

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:
    virtual JZNodePinWidget* createWidget(int id) override;
    virtual bool update(QString &error) override;
};

//JZNodeFunctionPointer
class JZCORE_EXPORT JZNodeFunctionPointer : public JZNode
{
public:
    JZNodeFunctionPointer();
    ~JZNodeFunctionPointer();

    void setFucntion(QString name);
    QString fucntion();
    
    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
        
protected:
};

//JZNodeDisplay
class JZCORE_EXPORT JZNodeDisplay : public JZNode
{
public:
    JZNodeDisplay();
    ~JZNodeDisplay();

    void addInput();
    void removeInput(int index);

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    virtual JZNodePinWidget* createWidget(int id) override;
    virtual QStringList pinActionList(int id) override;
    virtual bool pinActionTriggered(int id, int index) override;

protected:
    virtual bool canLink(int node_id,int pin_id,QString &error) override;
    virtual void onPinLinked(int id) override;
    virtual void onPinUnlinked(int id) override;
};

//JZNodePrint
class JZCORE_EXPORT JZNodePrint : public JZNode
{
public:
    JZNodePrint();
    ~JZNodePrint();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;   
protected:

};

//JZNodeCreate
class JZCORE_EXPORT JZNodeCreate : public JZNode
{
public:
    JZNodeCreate();
    ~JZNodeCreate();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;    
    virtual bool update(QString &error) override;

    void setClassName(const QString &name);
    QString className() const;

};

//JZNodeCreateFromString
class JZCORE_EXPORT JZNodeCreateFromString : public JZNode
{
public:
    JZNodeCreateFromString();
    ~JZNodeCreateFromString();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    virtual bool update(QString &error) override;

    void setClassName(const QString &name);
    QString className() const;

    void setContext(const QString &text);
    QString context() const;
};

//JZNodeParamThis
class JZCORE_EXPORT JZNodeThis : public JZNode
{
public:
    JZNodeThis();
    ~JZNodeThis();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual bool update(QString &error) override;
};

//JZNodeParam
class JZCORE_EXPORT JZNodeParam : public JZNode
{
public:
    JZNodeParam();
    ~JZNodeParam();        

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual bool update(QString &error) override;

    void setVariable(const QString &name);
    QString variable() const;

    virtual void drag(const QVariant &value);

protected:

};

//JZNodeSetParam
class JZCORE_EXPORT JZNodeSetParam : public JZNode
{
public:
    JZNodeSetParam();
    ~JZNodeSetParam();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual bool update(QString &error) override;
    
    void setVariable(const QString &name);
    QString variable() const;

    void setValue(const QString &name);
    QString value() const;

    virtual void drag(const QVariant &value) override;

protected:        

};

//JZNodeSetParamDataFlow
class JZCORE_EXPORT JZNodeSetParamDataFlow : public JZNode
{
public:
    JZNodeSetParamDataFlow();
    ~JZNodeSetParamDataFlow();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual bool update(QString &error) override;
    
    void setVariable(const QString &name);
    QString variable() const;
    
    void setValue(const QString &name);
    QString value() const;

    virtual void drag(const QVariant &value) override;

protected:    

};

//JZNodeAbstractMember
class JZCORE_EXPORT JZNodeAbstractMember : public JZNode
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
    int m_memberType;
};

//JZNodeMemberParam
class JZCORE_EXPORT JZNodeMemberParam : public JZNodeAbstractMember
{
public:
    JZNodeMemberParam();
    ~JZNodeMemberParam();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    bool update(QString &error);
};

//JZNodeSetMemberParam
class JZCORE_EXPORT JZNodeSetMemberParam : public JZNodeAbstractMember
{
public:
    JZNodeSetMemberParam();
    ~JZNodeSetMemberParam();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
    bool update(QString &error);
};

//JZNodeSetMemberParamData
class JZCORE_EXPORT JZNodeSetMemberParamData : public JZNodeAbstractMember
{
public:
    JZNodeSetMemberParamData();
    ~JZNodeSetMemberParamData();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

//JZNodeClone
class JZCORE_EXPORT JZNodeClone : public JZNode
{
public:
    JZNodeClone();
    ~JZNodeClone();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

//JZNodeSwap
class JZCORE_EXPORT JZNodeSwap : public JZNode
{
public:
    JZNodeSwap();
    ~JZNodeSwap();

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;
};

#endif
