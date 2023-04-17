#include "JZNodeExpression.h"
#include "JZNodeIR.h"
#include "JZNodeCompiler.h"

JZNodeOperator::JZNodeOperator()
{
    JZNodePin in1,in2,out;
    in1.setName("in1");
    in1.setFlag(Prop_in | Prop_param );
    
    in2.setName("in2");
    in2.setFlag(Prop_in | Prop_param);

    out.setName("out");
    out.setFlag(Prop_out | Prop_param);

    m_in1 = addProp(in1);
    m_in2 = addProp(in2);
    m_out = addProp(out);
}

bool JZNodeOperator::compiler(JZNodeCompiler *c,QString &error)
{
    int r1 = c->paramId(m_id,m_in1);
    int r2 = c->paramId(m_id,m_in2);
    int r3 = c->paramId(m_id,m_out);
        
    JZNodeIR ir(m_op);
    ir.params << QString::number(r3) << QString::number(r1) << QString::number(r2);
    c->addStatement(ir);    

    return true;
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
    
//JZNodeXor
JZNodeXor::JZNodeXor()
{
    m_name = "xor";
    m_type = Node_xor;
    m_op = OP_xor;
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
        JZNodePin pin;
        addProp(pin);
    }
    
    for(int i = 0; i < exp.outList.size(); i++)
    {
        JZNodePin pin;
        addProp(pin);
    }

    return true;
}