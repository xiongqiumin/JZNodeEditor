#include "JZNodeIR.h"
#include "JZNodeType.h"

//JZNodeIRParam
JZNodeIRParam::JZNodeIRParam()
{
    type = None;
    cache = nullptr;
}

bool JZNodeIRParam::isNull() const
{
    return type == None;
}

bool JZNodeIRParam::isLiteral() const
{
    return type == Literal;
}

bool JZNodeIRParam::isId() const
{
    return type == Id;
}

bool JZNodeIRParam::isReg() const
{
    return type == Id && id() >= Reg_Start;
}

bool JZNodeIRParam::isRef() const
{
    return type == Reference;
}

bool JZNodeIRParam::isThis() const
{
    return type == This;
}

int JZNodeIRParam::id() const
{
    Q_ASSERT(type == Id);
    return value.toInt();
}

QString JZNodeIRParam::ref() const
{
    Q_ASSERT(type == Reference);
    return value.toString();
}

const QVariant &JZNodeIRParam::literal() const
{
    Q_ASSERT(type == Literal);
    return value;
}

QDataStream &operator<<(QDataStream &s, const JZNodeIRParam &param)
{
    s << param.type;
    s << param.value;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeIRParam &param)
{
    s >> param.type;
    s >> param.value;
    return s;
}

JZNodeIRParam irRef(const QString &id)
{
    JZNodeIRParam param;
    param.type = JZNodeIRParam::Reference;
    param.value = id;
    return param;        
}

JZNodeIRParam irId(int id)
{
    Q_ASSERT(id >= 0);    

    JZNodeIRParam param;
    param.type = JZNodeIRParam::Id;
    param.value = id;
    return param;
}

JZNodeIRParam irLiteral(const QVariant &value)
{
    Q_ASSERT(JZNodeType::isLiteralType(JZNodeType::variantType(value)));
    
    JZNodeIRParam param;
    param.type = JZNodeIRParam::Literal;
    param.value = value;
    return param;
}

JZNodeIRParam irThis()
{
    JZNodeIRParam param;
    param.type = JZNodeIRParam::This;
    return param;
}

//JZNodeIR
JZNodeIR *createNodeIR(int type)
{
    switch (type)
    {
    case OP_nodeId:
        return new JZNodeIRNodeId();
    case OP_nop:
    case OP_return:
    case OP_exit:
    case OP_clearReg:
        return new JZNodeIR(type);
    case OP_alloc:
        return new JZNodeIRAlloc();
    case OP_add:
    case OP_sub:
    case OP_mul:
    case OP_div:
    case OP_mod:
    case OP_eq:
    case OP_ne:
    case OP_le:
    case OP_ge:
    case OP_lt:
    case OP_gt:
    case OP_and:
    case OP_or:
    case OP_not:
    case OP_bitand:
    case OP_bitor:
    case OP_bitxor:
    case OP_bitresver:
        return new JZNodeIRExpr(type);    
    case OP_set:    
        return new JZNodeIRSet();
    case OP_clone:
        return new JZNodeIRClone();
    case OP_watch:
        return new JZNodeIRWatch();
    case OP_convert:
        return new JZNodeIRConvert();
    case OP_jmp:
    case OP_je:
    case OP_jne:
        return new JZNodeIRJmp(type);
    case OP_call:       
        return new JZNodeIRCall();
    case OP_assert:
        return new JZNodeIRAssert();
    default:
        break;
    }

    Q_ASSERT(0);
    return nullptr;
}

JZNodeIR::JZNodeIR()
{
    type = OP_none;
    pc = -1;
}

JZNodeIR::JZNodeIR(int t)
{
    type = t;
    pc = -1;    
}

JZNodeIR::~JZNodeIR()
{

}

void JZNodeIR::saveToStream(QDataStream &s) const
{
    s << type;    
    s << pc;    
    s << memo;
}

void JZNodeIR::loadFromStream(QDataStream &s)
{
    s >> type;    
    s >> pc;  
    s >> memo;
}

//JZNodeIRNodeId
JZNodeIRNodeId::JZNodeIRNodeId()    
{
    type = OP_nodeId;
    id = -1;
    isBreakPoint = false;
}

JZNodeIRNodeId::~JZNodeIRNodeId()
{
    
}

void JZNodeIRNodeId::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << id;
}

void JZNodeIRNodeId::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> id;
}

//JZNodeIRAlloc
JZNodeIRAlloc::JZNodeIRAlloc()
{    
    type = OP_alloc;
    dataType = Type_none;
    allocType = None;
    id = -1;
}

JZNodeIRAlloc::~JZNodeIRAlloc()
{

}

void JZNodeIRAlloc::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << allocType << name << id << dataType;
}

void JZNodeIRAlloc::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> allocType >> name >> id >> dataType;
}

//JZNodeIRExpr
JZNodeIRExpr::JZNodeIRExpr(int ir_type)
    :JZNodeIR(ir_type)
{    
}

JZNodeIRExpr::~JZNodeIRExpr()
{
    
}

void JZNodeIRExpr::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << dst << src1 << src2;
}

void JZNodeIRExpr::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> dst >> src1 >> src2;
}

//JZNodeIRSet
JZNodeIRSet::JZNodeIRSet()    
{
    type = OP_set;
}

JZNodeIRSet::~JZNodeIRSet()
{
    
}

void JZNodeIRSet::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << src << dst;
}

void JZNodeIRSet::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> src >> dst;
}

//JZNodeIRClone
JZNodeIRClone::JZNodeIRClone()    
{
    type = OP_clone;
}

JZNodeIRClone::~JZNodeIRClone()
{
    
}

void JZNodeIRClone::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << src << dst;
}

void JZNodeIRClone::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> src >> dst;
}

//JZNodeIRWatch
JZNodeIRWatch::JZNodeIRWatch()
{
    type = OP_watch;
}

JZNodeIRWatch::~JZNodeIRWatch()
{

}

void JZNodeIRWatch::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << source << traget;
}

void JZNodeIRWatch::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> source >> traget;
}

//JZNodeIRConvert
JZNodeIRConvert::JZNodeIRConvert()
{
    type = OP_convert;
}

JZNodeIRConvert::~JZNodeIRConvert()
{
}

void JZNodeIRConvert::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << src << dst << dstType;
}

void JZNodeIRConvert::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> src >> dst >> dstType;
}   

//JZNodeIRJmp
JZNodeIRJmp::JZNodeIRJmp(int ir_type)
    :JZNodeIR(ir_type)
{
    Q_ASSERT(ir_type == OP_je || ir_type == OP_jne || ir_type == OP_jmp);    
    jmpPc = -1;
}

JZNodeIRJmp::~JZNodeIRJmp()
{
    
}

void JZNodeIRJmp::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << jmpPc;
}

void JZNodeIRJmp::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> jmpPc;
}

//JZNodeIRCall
JZNodeIRCall::JZNodeIRCall()
{
    type = OP_call;
    inCount = 0;
    cache = nullptr;
}

JZNodeIRCall::~JZNodeIRCall()
{
    
}

void JZNodeIRCall::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << function << inCount;
}

void JZNodeIRCall::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> function >> inCount;
}

//JZNodeIRAssert
JZNodeIRAssert::JZNodeIRAssert()
{
    type = OP_assert;
}

JZNodeIRAssert::~JZNodeIRAssert()
{

}

void JZNodeIRAssert::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << tips;
}

void JZNodeIRAssert::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> tips;
}