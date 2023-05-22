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

    QVariant literal() const;
    void setLiteral(QVariant value);

protected:
    int m_out;
    QVariant m_value;
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

//JZNodeParam
class JZNodeParam : public JZNode
{
public:
    JZNodeParam();
    ~JZNodeParam();        

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    QString paramId() const;
    void setParamId(QString paramId);

protected:
    int m_out;
    QString m_param;
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

    QString paramId() const;
    void setParamId(QString paramId);

protected:    
    QString m_param;
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

    QString paramId() const;
    void setParamId(QString paramId);

protected:
    QString m_param;
};

#endif
