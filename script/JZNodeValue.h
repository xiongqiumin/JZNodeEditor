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

    void setEnum(QString text);
    void setKey(QString text);
    void setValue(int value);
};

//JZNodeFlag
class JZNodeFlag : public JZNode
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
class JZNodeConvert : public JZNode
{
public:
    JZNodeConvert();
    ~JZNodeConvert();

    void setOutputType(int type);

    virtual bool compiler(JZNodeCompiler *compiler, QString &error) override;

protected:
    virtual JZNodePinWidget* createWidget(int id) override;
    virtual void onPinChanged(int id) override;
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

//JZNodeDisplay
class JZNodeDisplay : public JZNode
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
    virtual void onPinChanged(int id) override;

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
    virtual void onPinChanged(int id) override;

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
    virtual bool update(QString &error) override;
};

//JZNodeParam
class JZNodeParam : public JZNode
{
public:
    JZNodeParam();
    ~JZNodeParam();        

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual void onPinChanged(int id) override;

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
    virtual void onPinChanged(int id) override;
    
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
    virtual void onPinChanged(int id) override;
    
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
    JZNodeAbstractMember();
    ~JZNodeAbstractMember();

    QString className();

    void setMember(QStringList params);    
    QStringList member();    

protected:
    virtual void onPinLinked(int id) override;
    void updateMemberType();
    void updateMemberType(int type);
    QList<int> memberPinList();
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
