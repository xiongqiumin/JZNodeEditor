#include <QPushButton>
#include "JZNodeOperator.h"
#include "JZNodeIR.h"
#include "JZNodeCompiler.h"
#include "JZRegExpHelp.h"

JZNodeOperator::JZNodeOperator(int node_type, JZNodeIRType op_type)
{
    m_type = node_type;
    m_op = op_type;
    int in1 = addParamIn("");
    int in2 = addParamIn("");
    setPinTypeNumber(in1);
    setPinTypeNumber(in2);
    pin(in1)->setValue("0");
    pin(in2)->setValue("0");
    addParamOut("out");    
}

int JZNodeOperator::op() const
{
    return m_op;
}

void JZNodeOperator::addInput()
{
    auto pin0 = pin(paramIn(0));
    int in = addParamIn("", pin0->flag());    
    pin(in)->setDataType(pin0->dataType());
}

void JZNodeOperator::removeInput(int index)
{
    int id = paramInList()[index];
    removePin(id);
}

bool JZNodeOperator::checkPinInput(JZNodeCompiler *c,QString &error)
{
    auto input_list = paramInList();
    if(!c->checkPinInType(m_id,input_list,error))
        return false;
  

    QList<int> has_input_type,all_input_type;
    for(int i = 0; i < input_list.size(); i++)
    {
        all_input_type << c->pinType(m_id,input_list[i]);
        if(c->pinInputCount(m_id,input_list[i]) != 0)
            has_input_type << c->pinType(m_id,input_list[i]);
    }
    
    auto env = environment();
    int in_type = Type_none;
    if(has_input_type.size() > 0)
        in_type = env->upType(has_input_type);
    else
        in_type = env->upType(all_input_type);
    
    if(in_type == Type_none)
    {
        QStringList strs;
        for(int i = 0; i < input_list.size(); i++)
        {
            strs << env->typeToName(all_input_type[i]);
        }
        error = "无法对" + strs.join(",") + "进行运算";
        return false;
    }

    for(int i = 0; i < input_list.size(); i++)
        c->setPinType(m_id,input_list[i],in_type);
        
    return true;
}

bool JZNodeOperator::compiler(JZNodeCompiler *c,QString &error)
{
    if(!checkPinInput(c,error))
        return false;

    if(!c->addDataInput(m_id,error))
        return false;

    calcPinOutType(c);
    auto input_list = paramInList();
    for(int i = 0; i < input_list.size() - 1; i++)
    {
        int in1 = 0;
        if(i == 0)
            in1 = paramIn(i);
        else
            in1 = paramOut(0);

        int in2 = paramIn(i+1);
        int out = paramOut(0);

        int r1 = c->paramId(m_id,in1);
        int r2 = c->paramId(m_id,in2);
        int r3 = c->paramId(m_id,out);
        c->addExpr(irId(r3),irId(r1),irId(r2),m_op);
    }
    return true;
}

void JZNodeOperator::calcPinOutType(JZNodeCompiler *c)
{        
    auto env = environment();
    int out_type = Type_none;    
    switch (m_op)
    {
        case OP_add:
        case OP_sub:
        case OP_mul:
        case OP_div:
        case OP_mod:
        case OP_bitand:
        case OP_bitor:
        case OP_bitxor:
        {
            auto list = paramInList();
            QList<int> in_types;
            for (int i = 0; i < list.size(); i++)            
                in_types << c->pinType(m_id, list[i]);

            if (m_op == OP_add && in_types[0] == Type_string)
                out_type = Type_string;
            else
                out_type = env->upType(in_types);
            break;
        }
        case OP_eq:
        case OP_ne:
        case OP_le:
        case OP_ge:
        case OP_lt:
        case OP_gt:
        case OP_and:
        case OP_or:
            out_type = Type_bool;
            break;
        default:
            Q_ASSERT(0);
            break;
    }    
    c->setPinType(m_id, paramOut(0), out_type);
}

//JZNodeAdd
JZNodeAdd::JZNodeAdd()
    :JZNodeOperator(Node_add,OP_add)
{    
    QStringList type = { JZNodeType::typeName(Type_int), JZNodeType::typeName(Type_int64), 
        JZNodeType::typeName(Type_double), JZNodeType::typeName(Type_string)};  
    
    m_name = "+";
    setPinType(paramIn(0),type);
    setPinType(paramIn(1),type);
    setPinType(paramOut(0),type);
}

//JZNodeSub
JZNodeSub::JZNodeSub()
    :JZNodeOperator(Node_sub,OP_sub)
{
    m_name = "-";    
    setPinTypeNumber(paramIn(0));
    setPinTypeNumber(paramIn(1));
    setPinTypeNumber(paramOut(0));
}
    
//JZNodeMul
JZNodeMul::JZNodeMul()
    :JZNodeOperator(Node_mul,OP_mul)
{
    m_name = "*";
    setPinTypeNumber(paramIn(0));
    setPinTypeNumber(paramIn(1));
    setPinTypeNumber(paramOut(0));
}

//JZNodeDiv
JZNodeDiv::JZNodeDiv()
    :JZNodeOperator(Node_div,OP_div)
{
    m_name = "/";
    setPinTypeNumber(paramIn(0));
    setPinTypeNumber(paramIn(1));
    setPinTypeNumber(paramOut(0));
    setParamInValue(1, "1");
}

//JZNodeMod
JZNodeMod::JZNodeMod()
    :JZNodeOperator(Node_mod,OP_mod)
{
    m_name = "%";
    setPinTypeNumber(paramIn(0));
    setPinTypeNumber(paramIn(1));
    setPinTypeNumber(paramOut(0));
}

//JZNodeBitAnd
JZNodeBitAnd::JZNodeBitAnd()
    :JZNodeOperator(Node_bitand,OP_bitand)
{
    m_name = "bit and";
    setPinTypeInt(paramIn(0));
    setPinTypeInt(paramIn(1));
    setPinTypeInt(paramOut(0));
}

//JZNodeBitOr
JZNodeBitOr::JZNodeBitOr()
    :JZNodeOperator(Node_bitor,OP_bitor)
{
    m_name = "bit or";
    setPinTypeInt(paramIn(0));
    setPinTypeInt(paramIn(1));
    setPinTypeInt(paramOut(0));
}

//JZNodeBitXor
JZNodeBitXor::JZNodeBitXor()
    :JZNodeOperator(Node_bitxor,OP_bitxor)
{
    m_name = "bit xor";
    setPinTypeInt(paramIn(0));
    setPinTypeInt(paramIn(1));
    setPinTypeInt(paramOut(0));
}

//JZNodeLE
JZNodeLE::JZNodeLE()
    :JZNodeOperator(Node_le, OP_le)
{
    QStringList cmp_type = { JZNodeType::typeName(Type_int),JZNodeType::typeName(Type_int64),
        JZNodeType::typeName(Type_double),JZNodeType::typeName(Type_string)};

    m_name = "<=";
    setPinType(paramIn(0), cmp_type);
    setPinType(paramIn(1), cmp_type);
    setPinTypeBool(paramOut(0));
}

//JZNodeGE
JZNodeGE::JZNodeGE()
    :JZNodeOperator(Node_ge, OP_ge)
{
    QStringList cmp_type = { JZNodeType::typeName(Type_int),JZNodeType::typeName(Type_int64),
        JZNodeType::typeName(Type_double),JZNodeType::typeName(Type_string)};

    m_name = ">=";
    setPinType(paramIn(0), cmp_type);
    setPinType(paramIn(1), cmp_type);
    setPinTypeBool(paramOut(0));
}

//JZNodeLT
JZNodeLT::JZNodeLT()
    :JZNodeOperator(Node_lt, OP_lt)
{
    QStringList cmp_type = { JZNodeType::typeName(Type_int),JZNodeType::typeName(Type_int64),
        JZNodeType::typeName(Type_double),JZNodeType::typeName(Type_string)};

    m_name = "<";
    setPinType(paramIn(0), cmp_type);
    setPinType(paramIn(1), cmp_type);
    setPinTypeBool(paramOut(0));
}

//JZNodeGT
JZNodeGT::JZNodeGT()
    :JZNodeOperator(Node_gt, OP_gt)
{
    QStringList cmp_type = { JZNodeType::typeName(Type_int),JZNodeType::typeName(Type_int64),
        JZNodeType::typeName(Type_double),JZNodeType::typeName(Type_string)};

    m_name = ">";
    setPinType(paramIn(0), cmp_type);
    setPinType(paramIn(1), cmp_type);
    setPinTypeBool(paramOut(0));
}

//JZNodeEQ
JZNodeEQ::JZNodeEQ()
    :JZNodeOperator(Node_eq, OP_eq)
{    
    m_name = "==";    
    setPinTypeArg(paramIn(0));
    setPinTypeArg(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeNE
JZNodeNE::JZNodeNE()
    :JZNodeOperator(Node_ne, OP_ne)
{    
    m_name = "!=";    
    setPinTypeArg(paramIn(0));
    setPinTypeArg(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeAnd
JZNodeAnd::JZNodeAnd()
    :JZNodeOperator(Node_and, OP_and)
{
    m_name = "and";
    setPinTypeBool(paramIn(0));
    setPinTypeBool(paramIn(1));
    setPinTypeBool(paramOut(0));
    pin(paramIn(0))->setFlag(Pin_in | Pin_param);
    pin(paramIn(1))->setFlag(Pin_in | Pin_param);
}

bool JZNodeAnd::compiler(JZNodeCompiler *c, QString &error)
{
    auto input_list = paramInList();    

    QVector<JZNodeIRJmp*> jmpList;
    int out = c->paramId(m_id, paramOut(0));
    c->addSetVariable(irId(out), irLiteral(false));
    for (int i = 0; i < input_list.size(); i++)
    {
        if (!c->addFlowInput(m_id, { input_list[i]}, error))
            return false;
        c->addNodeEnter(m_id);

        int in_id = c->paramId(m_id, input_list[i]);
        c->addCompare(irId(in_id), irLiteral(false), OP_eq);
        JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_je);
        c->addStatement(JZNodeIRPtr(jmp));
        jmpList.push_back(jmp);

        if (i == input_list.size() - 1)
            c->addSetVariable(irId(out), irLiteral(true));
    }
    int ret = c->addNop();
    for (int i = 0; i < jmpList.size(); i++)
        jmpList[i]->jmpPc = ret;
    return true;
}

//JZNodeOr
JZNodeOr::JZNodeOr()
    :JZNodeOperator(Node_or, OP_or)
{
    m_name = "or";
    setPinTypeBool(paramIn(0));
    setPinTypeBool(paramIn(1));
    setPinTypeBool(paramOut(0));
    pin(paramIn(0))->setFlag(Pin_in | Pin_param);
    pin(paramIn(1))->setFlag(Pin_in | Pin_param);
}

bool JZNodeOr::compiler(JZNodeCompiler *c, QString &error)
{
    auto input_list = paramInList();    

    QVector<JZNodeIRJmp*> jmpList;
    int out = c->paramId(m_id, paramOut(0));
    c->addSetVariable(irId(out), irLiteral(true));
    for (int i = 0; i < input_list.size(); i++)
    {
        if (!c->addFlowInput(m_id, { input_list[i] }, error))
            return false;
        c->addNodeEnter(m_id);

        int in_id = c->paramId(m_id, input_list[i]);
        c->addCompare(irId(in_id), irLiteral(true), OP_eq);
        JZNodeIRJmp *jmp = new JZNodeIRJmp(OP_je);
        c->addStatement(JZNodeIRPtr(jmp));
        jmpList.push_back(jmp);

        if (i == input_list.size() - 1)
            c->addSetVariable(irId(out), irLiteral(false));
    }
    int ret = c->addNop();
    for (int i = 0; i < jmpList.size(); i++)
        jmpList[i]->jmpPc = ret;
    return true;
}


//JZNodeBitReverse
JZNodeBitReverse::JZNodeBitReverse()
{
    m_name = "bit reverse";
    m_type = Node_bitreverse;
    int in = addParamIn("input");
    int out = addParamOut("reverse");
    setPinTypeInt(in);
    setPinTypeInt(out);
}

bool JZNodeBitReverse::compiler(JZNodeCompiler* c, QString& error)
{
    if (!c->addDataInput(m_id, error))
        return false;
    
    int in_type = c->pinType(m_id,paramIn(0));
    c->setPinType(m_id, paramOut(0),in_type);

    int id_in = c->paramId(m_id, paramIn(0));
    int id_out = c->paramId(m_id, paramOut(0));
    c->addSingleExpr(irId(id_out), irId(id_in), OP_bitreverse);

    return true;
}

//JZNodeNot
JZNodeNot::JZNodeNot()
{
    m_name = "not";
    m_type = Node_not;
    int in = addParamIn("input");
    int out = addParamOut("not");
    setPinTypeBool(in);
    setPinTypeBool(out);
}

bool JZNodeNot::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addDataInput(m_id, error))
        return false;

    int id_in = c->paramId(m_id, paramIn(0));
    int id_out = c->paramId(m_id, paramOut(0));
    c->addSingleExpr(irId(id_out), irId(id_in), OP_not);
    return true;
}

//JZNodeNeg
JZNodeNeg::JZNodeNeg()
{
    m_name = "neg";
    m_type = Node_not;
    int in = addParamIn("input");
    int out = addParamOut("neg");
    setPinType(in, { "int8","int16","int","int64" });
    setPinType(out, { "int8","int16","int","int64" });
}

bool JZNodeNeg::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addDataInput(m_id, error))
        return false;

    int in_type = c->pinType(m_id,paramIn(0));
    c->setPinType(m_id, paramOut(0),in_type);

    int id_in = c->paramId(m_id, paramIn(0));
    int id_out = c->paramId(m_id, paramOut(0));
    c->addSingleExpr(irId(id_out), irId(id_in), OP_neg);
    c->setPinType(m_id, paramOut(0),in_type);

    return true;
}