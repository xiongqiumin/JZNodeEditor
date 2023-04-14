#ifndef JZNODE_DATA_SOURCE_H_
#define JZNODE_DATA_SOURCE_H_

#include "JZNode.h"

//JZNodeValue
class JZNodeValue : public JZNode
{
public:
    JZNodeValue();
    ~JZNodeValue();        

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:            
};

//JZNodePrint
class JZNodePrint : public JZNode
{
public:
    JZNodePrint();
    ~JZNodePrint();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

//JZNodeSet
class JZNodeSet : public JZNode
{
public:
    JZNodeSet();
    ~JZNodeSet();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

//JZNodeGet
class JZNodeGet : public JZNode
{
public:
    JZNodeGet();
    ~JZNodeGet();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:
};

#endif
