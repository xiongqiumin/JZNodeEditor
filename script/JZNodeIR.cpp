#include "JZNodeIR.h"
#include "JZNodeType.h"

//JZNodeIRParam
JZNodeIRParam::JZNodeIRParam()
{
    m_id = -1;
    type = None;
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
    return (type == StackId || type == RegId);
}

bool JZNodeIRParam::isNodeId() const
{
    return (type == StackId && id() < Stack_User);
}

bool JZNodeIRParam::isStack() const
{
    return type == StackId;
}

bool JZNodeIRParam::isReg() const
{
    return type == RegId;
}

bool JZNodeIRParam::isRef() const
{
    return type == Reference;
}

bool JZNodeIRParam::isThis() const
{
    return type == This;
}

bool JZNodeIRParam::isIdRef() const
{
    return type == StackIdReference;
}

int JZNodeIRParam::id() const
{
    Q_ASSERT(type == StackId || type == RegId || type == StackIdReference);
    return m_id;
}

QString JZNodeIRParam::ref() const
{
    Q_ASSERT(type == Reference || type == StackIdReference);
    return m_ref;
}

const QVariant &JZNodeIRParam::literal() const
{
    Q_ASSERT(type == Literal);
    return m_literal;
}

QDataStream &operator<<(QDataStream &s, const JZNodeIRParam &param)
{
    s << param.type;
    s << param.m_id;
    s << param.m_ref;
    s << param.m_literal;
    return s;
}

QDataStream &operator>>(QDataStream &s, JZNodeIRParam &param)
{
    s >> param.type;
    s >> param.m_id;
    s >> param.m_ref;
    s >> param.m_literal;
    return s;
}

JZNodeIRParam irRef(const QString &ref)
{    
    if (ref == "this")
        return irThis();

    JZNodeIRParam param;
    param.type = JZNodeIRParam::Reference;
    param.m_ref = ref;
    return param;        
}

JZNodeIRParam irId(int id)
{
    Q_ASSERT(id >= 0);    

    JZNodeIRParam param;
    if(id < Reg_Start)
        param.type = JZNodeIRParam::StackId;
    else
        param.type = JZNodeIRParam::RegId;
    param.m_id = id;
    return param;
}

JZNodeIRParam irLiteral(const QVariant &value)
{
    Q_ASSERT(JZNodeType::isLiteralType(JZNodeType::variantType(value)));
    
    JZNodeIRParam param;
    param.type = JZNodeIRParam::Literal;
    param.m_literal = value;
    return param;
}

JZNodeIRParam irThis()
{
    JZNodeIRParam param;
    param.type = JZNodeIRParam::This;
    return param;
}

JZNodeIRParam irIdRef(int id, const QString& member)
{
    JZNodeIRParam param;
    param.type = JZNodeIRParam::StackIdReference;
    param.m_id = id;
    param.m_ref = member;
    return param;
}

//JZNodeIR
JZNodeIR *createNodeIR(JZNodeIRType type)
{
    switch (type)
    {
    case OP_nodeEnter:        
        return new JZNodeIRNodeEnter();
    case OP_nop:
    case OP_return:
    case OP_clearReg:
        return new JZNodeIR(type);
    case OP_alloc:
        return new JZNodeIRAlloc();
    case OP_free:
        return new JZNodeIRFree();
    case OP_reference:
        return new JZNodeIRReference();
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
    case OP_bitreverse:
        return new JZNodeIRExpr(type);    
    case OP_set:    
        return new JZNodeIRSet();
    case OP_clone:
        return new JZNodeIRClone();
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
    case OP_try:
        return new JZNodeIRTry();
    case OP_throw:
        return new JZNodeIRThrow();
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

JZNodeIR::JZNodeIR(JZNodeIRType t)
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

//JZNodeIRNodeEnter
JZNodeIRNodeEnter::JZNodeIRNodeEnter()
{
    type = OP_nodeEnter;
    id = -1;
}

JZNodeIRNodeEnter::~JZNodeIRNodeEnter()
{

}

void JZNodeIRNodeEnter::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << id;
}

void JZNodeIRNodeEnter::loadFromStream(QDataStream &s)
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
}

JZNodeIRAlloc::~JZNodeIRAlloc()
{

}

void JZNodeIRAlloc::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << allocType << dst << dataType;
}

void JZNodeIRAlloc::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> allocType >> dst >> dataType;
}

//JZNodeIRFree
JZNodeIRFree::JZNodeIRFree()
{
    type = OP_free;
    allocType = JZNodeIRAlloc::None;
}
JZNodeIRFree::~JZNodeIRFree()
{
}

void JZNodeIRFree::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << allocType << dst;
}
void JZNodeIRFree::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> allocType >> dst;
}

//JZNodeIRReference
JZNodeIRReference::JZNodeIRReference()
{
    type = OP_reference;
}

JZNodeIRReference::~JZNodeIRReference()
{
}

void JZNodeIRReference::saveToStream(QDataStream& s) const
{
    JZNodeIR::saveToStream(s);
    s << ref << orig;
}

void JZNodeIRReference::loadFromStream(QDataStream& s)
{
    JZNodeIR::loadFromStream(s);
    s >> ref >> orig;
}

//JZNodeIRExpr
JZNodeIRExpr::JZNodeIRExpr(JZNodeIRType ir_type)
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

//JZNodeIRConvert
JZNodeIRConvert::JZNodeIRConvert()
{
    type = OP_convert;
    dstType = Type_none;
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
JZNodeIRJmp::JZNodeIRJmp(JZNodeIRType ir_type)
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
    isVirtual = false;
}

JZNodeIRCall::~JZNodeIRCall()
{
    
}

void JZNodeIRCall::saveToStream(QDataStream &s) const
{
    JZNodeIR::saveToStream(s);
    s << function << inCount << isVirtual;
}

void JZNodeIRCall::loadFromStream(QDataStream &s)
{
    JZNodeIR::loadFromStream(s);
    s >> function >> inCount >> isVirtual;
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

//JZNodeIRTry
JZNodeIRTry::JZNodeIRTry()
{
    type = OP_try;
    catchType = InTry;
    catchPc = -1;
}
JZNodeIRTry::~JZNodeIRTry()
{

}

void JZNodeIRTry::saveToStream(QDataStream& s) const
{
    JZNodeIR::saveToStream(s);
    s << catchType << catchPc << irExcep;
}

void JZNodeIRTry::loadFromStream(QDataStream& s)
{
    JZNodeIR::loadFromStream(s);
    s >> catchPc >> catchPc >> irExcep;
}

//JZNodeIRThrow
JZNodeIRThrow::JZNodeIRThrow()
{
    type = OP_throw;
}
JZNodeIRThrow::~JZNodeIRThrow()
{

}

void JZNodeIRThrow::saveToStream(QDataStream& s) const
{
    JZNodeIR::saveToStream(s);
    s << exception;
}

void JZNodeIRThrow::loadFromStream(QDataStream& s)
{
    JZNodeIR::loadFromStream(s);
    s >> exception;
}

QByteArray NodeIRMagic()
{
    QByteArray result;
    QDataStream s(&result, QIODevice::WriteOnly);

    //node ir
    s << sizeof(JZNodeIRParam);
    s << sizeof(JZNodeIR);
    s << sizeof(JZNodeIRNodeEnter);
    s << sizeof(JZNodeIRCall);
    s << sizeof(JZNodeIRJmp);
    s << sizeof(JZNodeIRExpr);
    s << sizeof(JZNodeIRSet);
    s << sizeof(JZNodeIRTry);
    s << (int)OP_assert;

    return result;
}
