#include "JZNodeExpression.h"
#include "JZNodeIR.h"
#include "JZNodeCompiler.h"
#include "JZExpression.h"

JZNodeOperator::JZNodeOperator(int node_type,int op_type)
{
    Q_ASSERT(node_type >= Node_add && node_type <= Node_bitxor);
    Q_ASSERT(op_type >= OP_add && op_type <= OP_bitxor);

    m_type = node_type;
    m_op = op_type;
    int in1 = addParamIn("",Prop_editValue | Prop_dispValue);
    int in2 = addParamIn("",Prop_editValue | Prop_dispValue);
    prop(in1)->setValue(0);
    prop(in2)->setValue(0);
    addParamOut("out");

    JZNodePin btn;
    btn.setName("Add input");
    btn.setFlag(Prop_button | Prop_in | Prop_dispName);
    addProp(btn);
}

bool JZNodeOperator::pinClicked(int id)
{
    Q_UNUSED(id);
    int in = addParamIn("",Prop_editValue | Prop_dispValue);
    prop(in)->setValue(0);
    return true;
}

bool JZNodeOperator::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addDataInput(m_id,error))
        return false;
        
    auto input_list = paramInList();
    for(int i = 0; i < input_list.size(); i++)
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

QMap<int,int> JZNodeOperator::calcPropOutType(const QMap<int,int> &inType)
{
    int in1 = paramIn(0);
    int in2 = paramIn(1);
    int out = paramOut(0);

    QMap<int,int> result;
    switch (m_op)
    {
        case OP_add:
        case OP_sub:
        case OP_mul:
        case OP_div:
        case OP_mod:        
            result[out] = JZNodeType::calcExprType(inType[in1],inType[in2]);
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
            result[out] = Type_bool;
            break;
        default:
            Q_ASSERT(0);
            break;
    }
    return result;
}

//JZNodeAdd
JZNodeAdd::JZNodeAdd()
    :JZNodeOperator(Node_add,OP_add)
{
    m_name = "+";        
    setPinTypeNumber(paramIn(0));
    setPinTypeNumber(paramIn(1));
    setPinTypeNumber(paramOut(0));
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
    
//JZNodeEQ
JZNodeEQ::JZNodeEQ()
    :JZNodeOperator(Node_eq,OP_eq)
{
    m_name = "==";
    setPinTypeAny(paramIn(0));
    setPinTypeAny(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeNE
JZNodeNE::JZNodeNE()
    :JZNodeOperator(Node_ne,OP_ne)
{
    m_name = "!=";
    setPinTypeAny(paramIn(0));
    setPinTypeAny(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeLE
JZNodeLE::JZNodeLE()
    :JZNodeOperator(Node_le,OP_le)
{
    m_name = "<=";
    setPinTypeAny(paramIn(0));
    setPinTypeAny(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeGE
JZNodeGE::JZNodeGE()
    :JZNodeOperator(Node_ge,OP_ge)
{
    m_name = ">=";
    setPinTypeAny(paramIn(0));
    setPinTypeAny(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeLT
JZNodeLT::JZNodeLT()
    :JZNodeOperator(Node_lt,OP_lt)
{
    m_name = "<";
    setPinTypeAny(paramIn(0));
    setPinTypeAny(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeGT
JZNodeGT::JZNodeGT()
    :JZNodeOperator(Node_gt,OP_gt)
{
    m_name = ">";
    setPinTypeAny(paramIn(0));
    setPinTypeAny(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeAnd
JZNodeAnd::JZNodeAnd()
    :JZNodeOperator(Node_and,OP_and)
{
    m_name = "and";
    setPinTypeBool(paramIn(0));
    setPinTypeBool(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeOr
JZNodeOr::JZNodeOr()
    :JZNodeOperator(Node_or,OP_or)
{
    m_name = "or";
    setPinTypeBool(paramIn(0));
    setPinTypeBool(paramIn(1));
    setPinTypeBool(paramOut(0));
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

//JZNodeExpression
JZNodeExpression::JZNodeExpression()
{
    m_type = Node_expr;
    m_name = "expr";

    m_opMap["+"] = OP_add;
    m_opMap["-"] = OP_sub;
    m_opMap["*"] = OP_mul;
    m_opMap["/"] = OP_div;
    m_opMap["%"] = OP_mod;
}

bool JZNodeExpression::setExpr(QString expr,QString &error)
{
    setName(expr);
    m_propList.clear();
    m_exprList.clear();
    m_expression = expr;
    JZExpression parser;   
    if(!parser.parse(expr))
    {
        error = parser.error();
        return false;
    }

    m_exprList = parser.opList();
    for(int i = 0; i < parser.inList.size(); i++)
    {     
        addParamIn(parser.inList[i]);
    }    
    for(int i = 0; i < parser.outList.size(); i++)
    {
        addParamOut(parser.outList[i]);
    }

    return true;
}

QString JZNodeExpression::expr()
{
    return m_expression;
}

bool JZNodeExpression::compiler(JZNodeCompiler *c,QString &error)
{            
    QMap<int,int> reg_map;    
    auto toIr = [this,c,&reg_map](QString name)->JZNodeIRParam{
        JZNodeIRParam parma;        
        int id = -1;
        if(name.startsWith("#Reg"))
        {
            int reg_index = name.mid(4).toInt();
            if(!reg_map.contains(reg_index))
                reg_map[reg_index] = c->allocStack();
            id = reg_map[reg_index];                
        }
        else
        {
            auto pin = prop(name);
            Q_ASSERT(pin);
            id = c->paramId(m_id,pin->id());
        }
        return irId(id);
    };

    if(!c->addDataInput(m_id,error))
        return false;
        
    for(int op = 0; op < m_exprList.size(); op++)
    {
        QStringList strs = m_exprList[op].split(" ");
        if(strs[2].startsWith("@")) //func
        {
            int s = strs[2].indexOf("(");

            QString function = strs[2].mid(1,s - 1);
            QStringList params = strs[2].mid(s + 1,strs[2].size()-1 - (s + 1)).split(",");
            const FunctionDefine *define = c->function(function);
            if(!define)
            {
                error = "no such function " + function;
                return false;
            }
            if(define->paramIn.size() != params.size())
            {
                error = QString("Function %0 pamram error, expert %1 give %2.").arg(function,
                    QString::number(define->paramIn.size()),QString::number(params.size()));
                return false;
            }
            if(define->paramOut.size() != 1)
            {
                error = QString("Function %0 return error").arg(function);
                return false;
            }

            QList<JZNodeIRParam> paramIn;            
            for(int i = 0; i < params.size(); i++)
                paramIn << toIr(params[i]);
                
            QList<JZNodeIRParam> paramOut;            
            paramOut << toIr(strs[0]);
            c->addCall(irLiteral(function),paramIn,paramOut);
        }
        else
        {
            if(strs.size() == 5)
                c->addExpr(toIr(strs[0]),toIr(strs[2]),toIr(strs[4]),m_opMap[strs[3]]);
            else
                c->addSetVariable(toIr(strs[0]),toIr(strs[2]));
        }
    }

    return true;
}
