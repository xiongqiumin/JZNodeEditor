#include <QRegularExpression>
#include <QPushButton>
#include "JZNodeExpression.h"
#include "JZNodeIR.h"
#include "JZNodeCompiler.h"
#include "JZExpression.h"
#include "JZRegExpHelp.h"

JZNodeOperator::JZNodeOperator(int node_type,int op_type)
{
    m_type = node_type;
    m_op = op_type;
    int in1 = addParamIn("",Pin_editValue | Pin_dispValue);
    int in2 = addParamIn("",Pin_editValue | Pin_dispValue);
    setPinTypeInt(in1);
    setPinTypeInt(in2);
    pin(in1)->setValue("0");
    pin(in2)->setValue("0");
    addParamOut("out");    
}

void JZNodeOperator::addInputButton()
{
    addWidgetIn("Add input");
}

void JZNodeOperator::addInput()
{
    auto pin0 = pin(paramIn(0));
    int in = addParamIn("", pin0->flag());    
    pin(in)->setDataType(pin0->dataType());
    if(pin0->isEditValue())
        pin(in)->setValue(0);
}

void JZNodeOperator::removeInput(int index)
{
    int id = paramInList()[index];
    removePin(id);
}

QWidget* JZNodeOperator::createWidget(int id)
{
    Q_UNUSED(id);    
    QPushButton *btn = new QPushButton("Add Input");
    btn->adjustSize();
    btn->connect(btn, &QPushButton::clicked, [this] {
        QByteArray old = toBuffer();
        addInput();
        propertyChangedNotify(old);        
    });                
    return btn;
}

QStringList JZNodeOperator::pinActionList(int id)
{
    QStringList ret;
    if (paramInCount() > 2)
        ret.push_back("删除");

    return ret;
}

bool JZNodeOperator::pinActionTriggered(int id, int index)
{
    int pin_index = paramInList().indexOf(id);
    removeInput(pin_index);
    return true;
}

bool JZNodeOperator::compiler(JZNodeCompiler *c,QString &error)
{
    if(!c->addDataInput(m_id,error))
        return false;
        
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
    calcPropOutType(c);
    return true;
}

void JZNodeOperator::calcPropOutType(JZNodeCompiler *c)
{        
    int out_type = Type_none;    
    switch (m_op)
    {
        case OP_add:
        case OP_sub:
        case OP_mul:
        case OP_div:
        case OP_mod:
        {
            auto list = paramInList();
            QList<int> in_types;
            for (int i = 0; i < list.size(); i++)            
                in_types << c->pinType(m_id, list[i]);

            if (m_op == OP_add && in_types[0] == Type_string)
                out_type = Type_string;
            else
                out_type = JZNodeType::upType(in_types);
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
        case OP_bitand:
        case OP_bitor:
        case OP_bitxor:
            out_type = Type_int;
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
    m_name = "+";        
    setPinTypeNumber(paramIn(0));
    setPinTypeNumber(paramIn(1));
    setPinTypeNumber(paramOut(0));
    addInputButton();
}

//JZNodeSub
JZNodeSub::JZNodeSub()
    :JZNodeOperator(Node_sub,OP_sub)
{
    m_name = "-";    
    setPinTypeNumber(paramIn(0));
    setPinTypeNumber(paramIn(1));
    setPinTypeNumber(paramOut(0));
    addInputButton();
}
    
//JZNodeMul
JZNodeMul::JZNodeMul()
    :JZNodeOperator(Node_mul,OP_mul)
{
    m_name = "*";
    setPinTypeNumber(paramIn(0));
    setPinTypeNumber(paramIn(1));
    setPinTypeNumber(paramOut(0));
    addInputButton();
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
    addInputButton();
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
    

//JZNodeFloatEQ
JZNodeFloatEQ::JZNodeFloatEQ()
{
    m_name = "==";
    addParamIn("");
    addParamIn("");
    addParamIn("eps", Pin_dispName | Pin_editValue);

    setPinType(paramIn(0), {Type_double});
    setPinType(paramIn(1), {Type_double});
    setPinTypeBool(paramOut(0));
}

//JZNodeFloatNE
JZNodeFloatNE::JZNodeFloatNE()    
{
    m_name = "!=";
    addParamIn("");
    addParamIn("");
    addParamIn("eps", Pin_dispName | Pin_editValue);
    
    setPinType(paramIn(0), { Type_double });
    setPinType(paramIn(1), { Type_double });
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

//JZNodeLE
JZNodeLE::JZNodeLE()
    :JZNodeOperator(Node_le, OP_le)
{
    m_name = "<=";
    setPinType(paramIn(0), { Type_int,Type_double });
    setPinType(paramIn(1), { Type_int,Type_double });
    setPinTypeBool(paramOut(0));
}

//JZNodeGE
JZNodeGE::JZNodeGE()
    :JZNodeOperator(Node_ge, OP_ge)
{
    m_name = ">=";
    setPinType(paramIn(0), { Type_int,Type_double });
    setPinType(paramIn(1), { Type_int,Type_double });
    setPinTypeBool(paramOut(0));
}

//JZNodeLT
JZNodeLT::JZNodeLT()
    :JZNodeOperator(Node_lt, OP_lt)
{
    m_name = "<";
    setPinType(paramIn(0), { Type_int,Type_double });
    setPinType(paramIn(1), { Type_int,Type_double });
    setPinTypeBool(paramOut(0));
}

//JZNodeGT
JZNodeGT::JZNodeGT()
    :JZNodeOperator(Node_gt, OP_gt)
{
    m_name = ">";
    setPinType(paramIn(0), { Type_int,Type_double });
    setPinType(paramIn(1), { Type_int,Type_double });
    setPinTypeBool(paramOut(0));
}

//JZNodeEQ
JZNodeEQ::JZNodeEQ()
    :JZNodeOperator(Node_eq, OP_eq)
{    
    m_name = "==";    
    setPinTypeAny(paramIn(0));
    setPinTypeAny(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeNE
JZNodeNE::JZNodeNE()
    :JZNodeOperator(Node_ne, OP_ne)
{    
    m_name = "!=";    
    setPinTypeAny(paramIn(0));
    setPinTypeAny(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeStringAdd
JZNodeStringAdd::JZNodeStringAdd()
    :JZNodeOperator(Node_stringAdd, OP_add)
{
    m_name = "+";
    setPinTypeString(paramIn(0));
    setPinTypeString(paramIn(1));
    setPinTypeString(paramOut(0));
    addInputButton();
}

//JZNodeStringEQ
JZNodeStringEQ::JZNodeStringEQ()
    :JZNodeOperator(Node_stringEq, OP_eq)
{
    m_name = "==";
    setPinTypeString(paramIn(0));
    setPinTypeString(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeStringNE
JZNodeStringNE::JZNodeStringNE()
    :JZNodeOperator(Node_stringNe, OP_ne)
{
    m_name = "!=";
    setPinTypeString(paramIn(0));
    setPinTypeString(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeStringLE
JZNodeStringLE::JZNodeStringLE()
    :JZNodeOperator(Node_stringLe, OP_le)
{
    m_name = "<=";
    setPinTypeString(paramIn(0));
    setPinTypeString(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeStringGE
JZNodeStringGE::JZNodeStringGE()
    :JZNodeOperator(Node_stringGe, OP_ge)
{
    m_name = ">=";
    setPinTypeString(paramIn(0));
    setPinTypeString(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeStringLT
JZNodeStringLT::JZNodeStringLT()
    :JZNodeOperator(Node_stringLt, OP_lt)
{
    m_name = "<";
    setPinTypeString(paramIn(0));
    setPinTypeString(paramIn(1));
    setPinTypeBool(paramOut(0));
}

//JZNodeStringGT
JZNodeStringGT::JZNodeStringGT()
    :JZNodeOperator(Node_stringGt, OP_gt)
{
    m_name = ">";
    setPinTypeString(paramIn(0));
    setPinTypeString(paramIn(1));
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
    addInputButton();
}

bool JZNodeAnd::compiler(JZNodeCompiler *c, QString &error)
{
    auto input_list = paramInList();
    c->addNodeStart(m_id);

    QVector<JZNodeIRJmp*> jmpList;
    int out = c->paramId(m_id, paramOut(0));
    c->addSetVariable(irId(out), irLiteral(false));
    for (int i = 0; i < input_list.size(); i++)
    {
        if (!c->addFlowInput(m_id, input_list[i], error))
            return false;

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
    addInputButton();
}

bool JZNodeOr::compiler(JZNodeCompiler *c, QString &error)
{
    auto input_list = paramInList();
    c->addNodeStart(m_id);

    QVector<JZNodeIRJmp*> jmpList;
    int out = c->paramId(m_id, paramOut(0));
    c->addSetVariable(irId(out), irLiteral(true));
    for (int i = 0; i < input_list.size(); i++)
    {
        if (!c->addFlowInput(m_id, input_list[i], error))
            return false;

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

//JZNodeNot
JZNodeNot::JZNodeNot()
{
    m_name = "not";
    m_type = Node_not;
    int in = addParamIn("", Pin_editValue | Pin_dispValue);
    int out = addParamOut("");
    setPinTypeBool(in);
    setPinTypeBool(out);
}

bool JZNodeNot::compiler(JZNodeCompiler *c, QString &error)
{
    if (!c->addDataInput(m_id, error))
        return false;

    int id_in = c->paramId(m_id, paramIn(0));
    int id_out = c->paramId(m_id, paramOut(0));
    c->addExpr(irId(id_out), irId(id_in), irLiteral(0), OP_not);
    return true;
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
    m_pinList.clear();
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
        int id = addParamIn(parser.inList[i]);
        setPinTypeNumber(id);
    }    
    for(int i = 0; i < parser.outList.size(); i++)
    {
        int id = addParamOut(parser.outList[i]);
        setPinTypeNumber(id);
    }
    return true;
}

QString JZNodeExpression::expr()
{
    return m_expression;
}

void JZNodeExpression::saveToStream(QDataStream &s) const
{
    JZNode::saveToStream(s);
    s << m_expression;
    s << m_exprList;
}

void JZNodeExpression::loadFromStream(QDataStream &s)
{
    JZNode::loadFromStream(s);
    s >> m_expression;
    s >> m_exprList;
}

bool JZNodeExpression::compiler(JZNodeCompiler *c,QString &error)
{            
    QMap<int,int> reg_map;    
    auto toIr = [this, c, &reg_map](QString name)->JZNodeIRParam {
        JZNodeIRParam parma;
        if (name.startsWith("#Reg"))
        {
            int reg_index = name.mid(4).toInt();
            if (!reg_map.contains(reg_index))
                reg_map[reg_index] = c->allocStack(Type_double);
            int id = reg_map[reg_index];
            return irId(id);
        }
        else if(JZRegExpHelp::isInt(name))
        {
            return irLiteral(name.toInt());
        }
        else 
        {
            auto ptr = pin(name);
            Q_ASSERT(ptr);
            int id = c->paramId(m_id, ptr->id());
            return irId(id);
        }        
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
            const JZFunctionDefine *define = c->function(function);
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
            c->addCall(function,paramIn,paramOut);
        }
        else
        {
            if(strs.size() == 5)
                c->addExpr(toIr(strs[0]),toIr(strs[2]),toIr(strs[4]),m_opMap[strs[3]]);
            else
                c->addSetVariable(toIr(strs[0]),toIr(strs[2]));
        }
    }

    auto out_list = paramOutList();
    for(int i = 0; i < out_list.size(); i++)
        c->setPinType(id(), out_list[i], Type_double);
    return true;
}
