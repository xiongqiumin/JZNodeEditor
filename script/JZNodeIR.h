#ifndef JZNODE_IR_H_
#define JZNODE_IR_H_

#include <QStringList>
#include <QVector>
#include <QVariant>

enum
{
    OP_none,
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
    OP_xor,
    OP_set,
    OP_setValue,
    OP_get,    
    OP_jmp,
    OP_je,
    OP_jne,
    OP_cmp,    
    OP_call,   
    OP_return, 
    OP_exit,    
};

class JZNodeIR
{
public:
    JZNodeIR();
    JZNodeIR(int type);

    int type;    
    QVariantList params;
    int source;
    QString memo;
};


#endif
