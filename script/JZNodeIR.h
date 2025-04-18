#ifndef JZNODE_IR_H_
#define JZNODE_IR_H_

#include <QStringList>
#include <QVector>
#include <QVariant>
#include <QSharedPointer>
#include <QDataStream>
#include "JZNodeFunctionDefine.h"

enum JZNodeIRType
{
    OP_none,
    OP_nodeId,
    OP_nop,            
    OP_alloc, 
    OP_clearReg,
    OP_set,
    OP_buffer,
    OP_clone,
    OP_convert,
    OP_jmp,
    OP_je,
    OP_jne,      
    OP_call,     
    OP_return, 
    OP_exit,    
    OP_add,
    OP_sub,
    OP_mul,
    OP_div,
    OP_mod,
    OP_bitand,
    OP_bitor,
    OP_bitxor,
    OP_bitreverse,
    OP_eq,  // ==
    OP_ne,  // !=
    OP_le,  // <=
    OP_ge,  // >=
    OP_lt,  // <
    OP_gt,  // >
    OP_and,
    OP_or,
    OP_not,        
    OP_assert,
    OP_try,
    OP_throw,
};

enum{    
    Stack_Node = 0,
    Stack_User = 1000000,    

    Reg_Start = 2000000,    
    Reg_Cmp = Reg_Start,
    Reg_CallIn,   //函数传递参数, 调用函数时将 RegCall 数据拷贝到 Stack_User    
    Reg_CallOut = Reg_CallIn + 16,
    Reg_End = Reg_CallOut + 16,
};

class JZNodeIRParam
{
public:
    enum ParamType{
        None,
        StackId,
        RegId,
        Literal,
        Reference,
        This,
    };    

    JZNodeIRParam();
    bool isNull() const;
    bool isLiteral() const;
    bool isRef() const;
    bool isNodeId() const;
    bool isStack() const;
    bool isReg() const;
    bool isThis() const;

    int id() const;
    QString ref() const;
    const QVariant &literal() const;

    ParamType type;
    QVariant value;
};
QDataStream &operator<<(QDataStream &s, const JZNodeIRParam &param);
QDataStream &operator>>(QDataStream &s, JZNodeIRParam &param);
JZNodeIRParam irRef(const QString &id);
JZNodeIRParam irId(int id);
JZNodeIRParam irLiteral(const QVariant &value);
JZNodeIRParam irThis();

class JZNodeIR
{
public:
    JZNodeIR();
    JZNodeIR(JZNodeIRType type);
    virtual ~JZNodeIR();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);    

    JZNodeIRType type;
    int pc;        
    QString memo;
};
typedef QSharedPointer<JZNodeIR> JZNodeIRPtr;
JZNodeIR *createNodeIR(JZNodeIRType type);

class JZNodeIRNodeId : public JZNodeIR
{
public:    
    JZNodeIRNodeId();
    virtual ~JZNodeIRNodeId();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);     

    int id;   
    int breakPointType;
};

class JZNodeIRAlloc : public JZNodeIR
{
public:
    enum{
        None,
        Heap,
        Stack,
        StackId,
    };

    JZNodeIRAlloc();
    virtual ~JZNodeIRAlloc();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    int allocType;
    JZNodeIRParam dst;
    int dataType;
};

class JZNodeIRExpr : public JZNodeIR
{
public:    
    JZNodeIRExpr(JZNodeIRType type);
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

class JZNodeIRClone : public JZNodeIR
{
public:
    JZNodeIRClone();
    virtual ~JZNodeIRClone();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);   

    JZNodeIRParam dst;
    JZNodeIRParam src;
};

class JZNodeIRBuffer : public JZNodeIR
{
public:
    JZNodeIRBuffer();
    virtual ~JZNodeIRBuffer();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    JZNodeIRParam id;
    QByteArray buffer;
};

class JZNodeIRConvert: public JZNodeIR
{
public:
    JZNodeIRConvert();
    virtual ~JZNodeIRConvert();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);   

    JZNodeIRParam dst;
    int dstType;
    JZNodeIRParam src;
};

class JZNodeIRJmp : public JZNodeIR
{
public:
    JZNodeIRJmp(JZNodeIRType type);
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
    int inCount;
    bool isVirtual;
};

class JZNodeIRAssert : public JZNodeIR
{
public:
    JZNodeIRAssert();
    virtual ~JZNodeIRAssert();

    virtual void saveToStream(QDataStream &s) const;
    virtual void loadFromStream(QDataStream &s);

    JZNodeIRParam tips;
};

class JZNodeIRTry : public JZNodeIR
{
public:
    enum {
        InTry,
        OutTry,
    };

    JZNodeIRTry();
    virtual ~JZNodeIRTry();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    int catchType;
    int catchPc;
    JZNodeIRParam irExcep;
};

class JZNodeIRThrow : public JZNodeIR
{
public:
    JZNodeIRThrow();
    virtual ~JZNodeIRThrow();

    virtual void saveToStream(QDataStream& s) const;
    virtual void loadFromStream(QDataStream& s);

    JZNodeIRParam exception;
};
QByteArray NodeIRMagic();

#endif
