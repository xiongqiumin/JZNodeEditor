#ifndef JZNODE_IR_H_
#define JZNODE_IR_H_

#include <QStringList>
#include <QVector>
#include <QVariant>
#include <QSharedPointer>
#include <QDataStream>

enum
{
    OP_none,
    OP_nodeId,
    OP_nop,
    OP_add,
    OP_sub,
    OP_mul,
    OP_div,
    OP_mod,    
    OP_eq,  // ==
    OP_ne,  // !=
    OP_le,  // <=
    OP_ge,  // >=
    OP_lt,  // <
    OP_gt,  // >
    OP_and,
    OP_or,
    OP_bitand,
    OP_bitor,
    OP_bitxor,
    OP_set,    
    OP_get,    
    OP_getRef,
    OP_jmp,
    OP_je,
    OP_jne,  
    OP_call,     
    OP_return, 
    OP_exit,    
};

class JZNodeIRParam
{
public:
    enum{
        None,
        Id,
        Literal,
        Reference,
    };    

    JZNodeIRParam();
    bool isLiteral() const;
    bool isRef() const;
    bool isId() const;

    int id() const;
    QString ref() const;

    int type;
    QVariant value;
};
QDataStream &operator<<(QDataStream &s, const JZNodeIRParam &param);
QDataStream &operator>>(QDataStream &s, JZNodeIRParam &param);
JZNodeIRParam irRef(QString id);
JZNodeIRParam irId(int id);
JZNodeIRParam irLiteral(QVariant value);

class JZNodeIR
{
public:
    JZNodeIR();
    JZNodeIR(int type);
    virtual ~JZNodeIR();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);    

    int type;    
    int pc;
    int source;
    QString memo;
};
typedef QSharedPointer<JZNodeIR> JZNodeIRPtr;
JZNodeIR *createNodeIR(int type);

class JZNodeIRNodeId : public JZNodeIR
{
public:    
    JZNodeIRNodeId();
    virtual ~JZNodeIRNodeId();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);     

    int id;   
};

class JZNodeIRExpr : public JZNodeIR
{
public:    
    JZNodeIRExpr(int type);
    virtual ~JZNodeIRExpr();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);    

    JZNodeIRParam dst;
    JZNodeIRParam src1;
    JZNodeIRParam src2;
};

class JZNodeIRSet : public JZNodeIR
{
public:
    JZNodeIRSet();
    virtual ~JZNodeIRSet();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);   

    JZNodeIRParam dst;
    JZNodeIRParam src;
};

class JZNodeIRJmp : public JZNodeIR
{
public:
    JZNodeIRJmp(int type);
    virtual ~JZNodeIRJmp();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);   

    int jmpPc;
};

class JZNodeIRCall : public JZNodeIR
{
public:
    JZNodeIRCall();
    virtual ~JZNodeIRCall();
    
    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);   

    QString function;
    QList<JZNodeIRParam> paramIn;
    QList<JZNodeIRParam> paramOut;
};

#endif
