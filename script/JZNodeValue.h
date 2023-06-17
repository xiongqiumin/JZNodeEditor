#ifndef JZNODE_DATA_SOURCE_H_
#define JZNODE_DATA_SOURCE_H_

#include "JZNode.h"

//JZNodeValue
class JZNodeLiteral : public JZNode
{
public:
    JZNodeLiteral();
    ~JZNodeLiteral();        

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

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

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

//JZNodeCreate
class JZNodeCreate : public JZNode
{
public:
    JZNodeCreate();
    ~JZNodeCreate();

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    void setClassName(QString name);
};

//JZNodeParam
class JZNodeParam : public JZNode
{
public:
    JZNodeParam();
    ~JZNodeParam();        

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    virtual void setVariable(const QString &name) override;
    virtual QString variable() const override;

protected:

};

//JZNodeParamThis
class JZNodeThis : public JZNode
{
public:
    JZNodeThis();
    ~JZNodeThis();

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
};

//JZNodeSetParam
class JZNodeSetParam : public JZNode
{
public:
    JZNodeSetParam();
    ~JZNodeSetParam();

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    virtual void setVariable(const QString &name) override;
    virtual QString variable() const override;

protected:        

};


//JZNodeSetParamData
class JZNodeSetParamData : public JZNode
{
public:
    JZNodeSetParamData();
    ~JZNodeSetParamData();

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
    virtual void setVariable(const QString &name) override;
    virtual QString variable() const override;

protected:

};

#endif
