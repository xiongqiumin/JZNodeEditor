#include "JZNodeExpression.h"
#include "JZNodeIR.h"
#include "JZNodeCompiler.h"

JZNodeOperator::JZNodeOperator()
{
    m_in1 = addParamIn("in1",Prop_edit);
    m_in2 = addParamIn("in2",Prop_edit);
    m_out = addParamOut("out");
}

bool JZNodeOperator::compiler(JZNodeCompiler *c,QString &error)
{
    c->addDataInput(m_id);
    int r1 = c->paramId(m_id,m_in1);
    int r2 = c->paramId(m_id,m_in2);
    int r3 = c->paramId(m_id,m_out);            
    c->addExpr(irId(r3),irId(r1),irId(r2),m_op);
    return true;
}

QMap<int,int> JZNodeOperator::calcPropOutType(const QMap<int,int> &inType)
{
    QMap<int,int> result;
    switch (m_type)
    {
        case OP_add:
        case OP_sub:
        case OP_mul:
        case OP_div:
        case OP_mod:
            result[m_out] = JZNodeType::calcExprType(inType[m_in1],inType[m_in2]);
            break;
        case OP_eq:
        case OP_ne:
        case OP_le:
        case OP_ge:
        case OP_lt:
        case OP_gt:
        case OP_and:
        case OP_or:
        case OP_bitand:
        case OP_bitor:
        case OP_bitxor:
            result[m_out] = Type_int;
            break;
        default:
            Q_ASSERT(0);
            break;
    }
    return result;
}

//JZNodeAdd
JZNodeAdd::JZNodeAdd()
{
    m_name = "+";
    m_type = Node_add;
    m_op = OP_add;
}

//JZNodeSub
JZNodeSub::JZNodeSub()
{
    m_name = "-";
    m_type = Node_sub;
    m_op = OP_sub;
}
    
//JZNodeMul
JZNodeMul::JZNodeMul()
{
    m_name = "*";
    m_type = Node_mul;
    m_op = OP_mul;
}

//JZNodeDiv
JZNodeDiv::JZNodeDiv()
{
    m_name = "/";
    m_type = Node_div;
    m_op = OP_div;
}

//JZNodeMod
JZNodeMod::JZNodeMod()
{
    m_name = "%";
    m_type = Node_mod;
    m_op = OP_mod;
}
    
//JZNodeEQ
JZNodeEQ::JZNodeEQ()
{
    m_name = "==";
    m_type = Node_eq;
    m_op = OP_eq;
}

//JZNodeNE
JZNodeNE::JZNodeNE()
{
    m_name = "!=";
    m_type = Node_ne;
    m_op = OP_ne;
}

//JZNodeLE
JZNodeLE::JZNodeLE()
{
    m_name = "<=";
    m_type = Node_le;
    m_op = OP_le;
}

//JZNodeGE
JZNodeGE::JZNodeGE()
{
    m_name = ">=";
    m_type = Node_ge;
    m_op = OP_ge;
}

//JZNodeLT
JZNodeLT::JZNodeLT()
{
    m_name = "<";
    m_type = Node_lt;
    m_op = OP_lt;
}

//JZNodeGT
JZNodeGT::JZNodeGT()
{
    m_name = ">";
    m_type = Node_gt;
    m_op = OP_gt;
}

//JZNodeAnd
JZNodeAnd::JZNodeAnd()
{
    m_name = "and";
    m_type = Node_and;
    m_op = OP_and;
}

//JZNodeOr
JZNodeOr::JZNodeOr()
{
    m_name = "or";
    m_type = Node_or;
    m_op = OP_or;
}

//JZNodeBitAnd
JZNodeBitAnd::JZNodeBitAnd()
{
    m_name = "bit and";
    m_type = Node_bitand;
    m_op = OP_bitand;
}

//JZNodeBitOr
JZNodeBitOr::JZNodeBitOr()
{
    m_name = "bit or";
    m_type = Node_bitor;
    m_op = OP_bitor;
}

//JZNodeBitXor
JZNodeBitXor::JZNodeBitXor()
{
    m_name = "bit xor";
    m_type = Node_bitxor;
    m_op = OP_bitxor;
}

//JZNodeExpression
JZNodeExpression::JZNodeExpression()
{

}

bool JZNodeExpression::compiler(JZNodeCompiler *compiler,QString &error)
{
    JZExpression exp;
    if(!exp.parse(expression,error))
        return false;
    
    for(int i = 0; i < exp.inList.size(); i++)
    {     
        addParamIn(exp.inList[i]);
    }
    
    for(int i = 0; i < exp.outList.size(); i++)
    {
        addParamOut(exp.inList[i]);
    }

    return true;
}
