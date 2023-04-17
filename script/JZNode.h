#ifndef JZNODE_H_
#define JZNODE_H_

#include <QDataStream>
#include <QSharedPointer>
#include "JZNodePin.h"
#include "JZNodeIR.h"

enum
{
    Node_none,
    Node_value,        
    Node_function,    
    Node_event,
    Node_add,
    Node_sub,
    Node_mul,
    Node_div,
    Node_mod,    
    Node_eq,  // ==
    Node_ne,  // !=
    Node_le,  // <=
    Node_ge,  // >=
    Node_lt,  // <
    Node_gt,  // >
    Node_and,
    Node_or,
    Node_xor,    
    Node_expr,
    Node_for,
    Node_while,
    Node_branch,
    Node_if,        
    Node_switch,
    Node_return,
    Node_exit,
    Node_parallel,    
    Node_view,
    Node_get,
    Node_set,
    Node_print,
};

//JZNodeGemo
struct JZNodeGemo
{
    JZNodeGemo();
    JZNodeGemo(int id, int prop);

    bool isNull() const;
    bool operator==(const JZNodeGemo &other) const;

    int nodeId;
    int propId;
};

//JZNodeConnect
class JZNodeConnect
{
public:
    JZNodeConnect();
    
    int id;
    JZNodeGemo from;
    JZNodeGemo to;
};
QDataStream &operator<<(QDataStream &s, const JZNodeConnect &param);
QDataStream &operator>>(QDataStream &s, JZNodeConnect &param);
JZNodeConnect parseLine(const QByteArray &buffer);
QByteArray formatLine(const JZNodeConnect &line);

class JZNodeCompiler;
class JZNodeGraphItem;
class JZNode
{
public:
    JZNode();
    virtual ~JZNode();

    QString name();
    void setName(QString name);

    int id();
    void setId(int id);
    int type();

    int addFlow(const JZNodePin &prop);
    int addProp(const JZNodePin &prop);
    void removeProp(int id);
    JZNodePin *prop(int id);
    int indexOfProp(int id) const;
    int indexOfPropByType(int id, int type) const;
    QVector<int> propInList(int flag) const;
    QVector<int> propOutList(int flag) const;
    QVector<int> propListByType(int flag) const;
    const QList<JZNodePin> &propList() const;
    int propCount(int flag);

    virtual void expandNode();
    virtual bool compiler(JZNodeCompiler *compiler,QString &error) = 0;
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);    

protected:
    int addPropInternal(const JZNodePin &prop);

    int m_id;
    int m_type;
    QString m_name;
    QList<JZNodePin> m_propList;
};
typedef QSharedPointer<JZNode> JZNodePtr;

class JZStatementNode : public JZNode
{
public:

};

class JZNodeInput : public JZNode
{
public:
    JZNodeInput();    

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:    
    int m_in;
    int m_paramIdx;
};

class JZNodeOutput : public JZNode
{
public:
    JZNodeOutput();    

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

protected:    
    int m_out;
    int m_paramIdx;
};

class JZNodeReturn : public JZNode
{
public:
    JZNodeReturn();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
};

class JZNodeExit : public JZNode
{
public:
    JZNodeExit();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;
};

class JZNodeParallel : public JZNode
{
public:
    JZNodeParallel();
};

class JZNodeParallelEnd: public JZNode
{
public:
    JZNodeParallelEnd();
};

class JZNodeFor: public JZNode
{
public:
    JZNodeFor();
};

class JZNodeWhile: public JZNode
{
public:
    JZNodeWhile();
};

class JZNodeIf : public JZNode
{
public:
    JZNodeIf();
};

class JZNodeBranch : public JZNode
{
public:
    JZNodeBranch();
};

class JZNodeFunction : public JZNode
{
public:
    JZNodeFunction();

    virtual bool compiler(JZNodeCompiler *compiler,QString &error) override;

    virtual void saveToStream(QDataStream &s) const override;
    virtual void loadFromStream(QDataStream &s) override;

    QString functionName;
};

#endif
